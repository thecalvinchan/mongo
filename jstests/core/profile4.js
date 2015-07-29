// Check debug information recorded for a query.

// special db so that it can be run in parallel tests
var stddb = db;
var db = db.getSisterDB("profile4");

db.dropAllUsers();
t = db.profile4;
t.drop();

function profileCursor() {
    return db.system.profile.find( { user:username + "@" + db.getName() } );
}

function lastOp() {
    p = profileCursor().sort( { $natural:-1 } ).next();
//    printjson( p );
    return p;
}

function checkLastOp( spec ) {
    p = lastOp();
    for( i in spec ) {
        s = spec[ i ];
        assert.eq( s[ 1 ], p[ s[ 0 ] ], s[ 0 ] );
    }
}

try {
    username = "jstests_profile4_user";
    db.createUser({user: username, pwd: "password", roles: jsTest.basicUserRoles});
    db.auth( username, "password" );
    
    db.setProfilingLevel(0);
    
    db.system.profile.drop();
    assert.eq( 0 , profileCursor().count() )
    
    db.setProfilingLevel(2);

    t.find().itcount();
    checkLastOp( [// TODO re-enable when SERVER-19566 is implemented.
                  // [ "op", "query" ],
                  // [ "query", {} ],
                  // [ "responseLength", 20 ],
                  [ "ns", "profile4.profile4" ],
                  [ "ntoreturn", 0 ],
                  [ "ntoskip", 0 ],
                  [ "nscanned", 0 ],
                  [ "keyUpdates", 0 ],
                  [ "nreturned", 0 ],
                  [ "cursorExhausted", true] ] );
    
    // check write lock stats are set
    t.save( {} );
    o = lastOp();
    assert.eq('insert', o.op);
    printjson(o.locks);
    assert.lt( 0, Object.keys(o.locks).length );

    // check read lock stats are set
    t.find();
    o = lastOp();
    // TODO re-enable when SERVER-19566 is implemented.
    // assert.eq('query', o.op);
    printjson(o.locks);
    assert.lt( 0, Object.keys(o.locks).length );

    t.save( {} );
    t.save( {} );
    t.find().skip( 1 ).limit( 4 ).itcount();
    checkLastOp( [// TODO re-enable when SERVER-19566 is implemented.
                  // [ "ntoreturn", 4 ],
                  [ "ntoskip", 1 ],
                  [ "nscannedObjects", 3 ],
                  [ "nreturned", 2 ] ] );

    t.find().batchSize( 2 ).next();
    o = lastOp();
    assert.lt( 0, o.cursorid );
    
    t.find( {a:1} ).itcount();
    // TODO re-enable when SERVER-19566 is implemented.
    // checkLastOp( [ [ "query", {a:1} ] ] );
    
    t.find( {_id:0} ).itcount();
    checkLastOp( [ [ "idhack", true ] ] );
    
    t.find().sort( {a:1} ).itcount();
    checkLastOp( [ [ "scanAndOrder", true ] ] );
    
    t.ensureIndex( {a:1} );
    t.find( {a:1} ).itcount();
    o = lastOp();
    assert.eq( "FETCH", o.execStats.stage, tojson( o.execStats ) );
    assert.eq( "IXSCAN", o.execStats.inputStage.stage, tojson( o.execStats ) );

    // For queries with a lot of stats data, the execution stats in the profile
    // is replaced by the plan summary.
    var orClauses = 32;
    var bigOrQuery = { $or: [] };
    for ( var i = 0; i < orClauses; ++i ) {
        var indexSpec = {};
        indexSpec[ "a" + i ] = 1;
        t.ensureIndex( indexSpec );
        bigOrQuery[ "$or" ].push( indexSpec );
    }
    t.find( bigOrQuery ).itcount();
    o = lastOp();
    assert.neq( undefined, o.execStats.summary, tojson( o.execStats ) );

    // Confirm "cursorExhausted" not set when cursor is open.
    t.drop();
    t.insert([{_id:0},{_id:1},{_id:2},{_id:3},{_id:4}]);
    t.find().batchSize(2).next(); // Query performed leaving open cursor
    var operation = lastOp();
    // TODO re-enable when SERVER-19566 is implemented.
    // assert.eq("query", operation.op);
    assert(!("cursorExhausted" in operation));

    var cursor = t.find().batchSize(2);
    cursor.next(); // Perform initial query and consume first of 2 docs returned.
    cursor.next(); // Consume second of 2 docs from initial query.
    cursor.next(); // getMore performed, leaving open cursor.
    operation = lastOp();
    // TODO re-enable when SERVER-19566 is implemented.
    // assert.eq("getmore", operation.op);
    assert(!("cursorExhausted" in operation));

    // Exhaust cursor and confirm getMore has "cursorExhausted:true".
    cursor.itcount();
    checkLastOp( [ [ "cursorExhausted", true] ] );

    db.setProfilingLevel(0);
    db.system.profile.drop();    
}
finally {
    db.setProfilingLevel(0);
    db = stddb;
}
