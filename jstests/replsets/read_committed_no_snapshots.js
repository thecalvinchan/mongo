// Test basic read committed maxTimeMS timeout while waiting for a committedSnapshot

load("jstests/replsets/rslib.js");

(function() {
"use strict";

// Set up a set and grab things for later.
var name = "read_committed_no_snapshots";
var replTest = new ReplSetTest({name: name,
                                nodes: 3,
                                nodeOptions: {setParameter: "enableReplSnapshotThread=true"}});
var nodes = replTest.nodeList();
replTest.startSet();
replTest.initiate({"_id": name,
                   "members": [
                       { "_id": 0, "host": nodes[0] },
                       { "_id": 1, "host": nodes[1], priority: 0 },
                       { "_id": 2, "host": nodes[2], arbiterOnly: true }],
                   "settings": {
                       "protocolVersion": 1
                   }
                  });

// Get connections and collection.
var primary = replTest.getPrimary();
var secondary = replTest.liveNodes.slaves[0];
var secondaryId = replTest.getNodeId(secondary);
var db = primary.getDB(name);

if (!db.serverStatus().storageEngine.supportsCommittedReads) {
    assert.neq(db.serverStatus().storageEngine.name, "wiredTiger");
    jsTest.log("skipping test since storage engine doesn't support committed reads");
    return;
}

// Do a write, wait for it to replicate, and ensure it is visible.
assert.writeOK(db.foo.save({_id: 1, state: 0}, {writeConcern: {w: "majority", wtimeout: 60*1000}}));

// We need to propagate the lastOp from the primary as afterOpTime in the secondary to ensure we
// wait for the write to be in the majority committed view.
// TODO SERVER-19890 Fix this to get the real optime rather than constructing a fake one.
var lastOp = {ts: db.getLastErrorObj().lastOp, term: 1};

secondary.setSlaveOk();
// Timeout is based on heartbeat timeout.
assert.commandWorked(secondary.getDB(name).foo.runCommand(
            'find', {"readConcern": {"level": "majority", "afterOpTime": lastOp},
                     "maxTimeMS": 10 * 1000}));

// Disable snapshotting via failpoint
secondary.adminCommand({configureFailPoint: 'disableSnapshotting', mode: 'alwaysOn'});

// Resync to drop any existing snapshots
secondary.adminCommand({resync: 1});
    
// Ensure maxTimeMS times out while waiting for this snapshot
assert.commandFailed(secondary.getDB(name).foo.runCommand(
            'find', {"readConcern": {"level": "majority"}, "maxTimeMS": 1000}));

// Reconfig to make the secondary the primary
var config = primary.getDB("local").system.replset.findOne();
config.members[0].priority = 0;
config.members[1].priority = 3;
config.version++;
primary = reconfig(replTest, config, true);

// Ensure maxTimeMS times out while waiting for this snapshot
assert.commandFailed(primary.getSiblingDB(name).foo.runCommand(
            'find', {"readConcern": {"level": "majority"}, "maxTimeMS": 1000}));
}());
