t = db.tinyjs_find;
t.drop();

t.save( { a : 1 , b : "hi" } );
t.save( { a : 2 , b : "hi" } );

assert( t.findOne( {a : 1} ).b == t.findOne( { $where: 'return this.a == 1;' } ).b, "A");

tests = []

// Document generation functions

/**
 * Helper function to generate documents in the collection using the
 * generator function to generate the documents
 */
function generateDocs(nDocs, generator) {
    return function(collection) {
        collection.drop();
        docs = [];
        for (var i = 0; i < nDocs; i++) {
          docs.push(generator());
        }
        collection.insert(docs, {'ordered': true});
    }
 }

/**
 * Generates simple docs with increasing x value
 */
function increasingXGenerator() {
  var x = 0;
  return function() {
      var doc = { "x" : x};
      x++;
      return doc;
  };
}

/**
 * Generates a documents containing an array of 10
 * random numbers between 0 and 100
 */
function arrayGenerator() {
    var results = [];
    for (var j = 0; j < 10; j++) {
      results.push(Math.floor(Math.random() * 101));
    }
    return {"results": results };
}

/**
 * Generates documents of the form {x: i, y: j}
 * with increasing values for x and y
 * y will cycle from 0 to numY.
 */
 function tupleGenerator(numY) {
    var x = 0;
    var y = 0;
    return function() {
      var doc = { "x" : x, "y": y};
      if (y++ > numY) {
        y = 0;
        x++;
      }
      return doc;
    };
 }

/**
 * Generates documents containing 4-letter strings
 */
function permutationGenerator() {
    var strings = [];
    for (var i = 0; i < 26; i++) {
        strings.push(String.fromCharCode(97+i));
    }
    var i = 0;
    var j = 0;
    var k = 0;
    var l = 0;
    return function() {
      var doc = {x: strings[i]+strings[j]+strings[k]+strings[l]};
      if (++l > 25) {
        l = 0;
        if (++k > 25) {
            k = 0;
            if (++j > 25) {
                j = 0;
                if (++i > 25) {
                    i = 0;
                }
            }
        }
      }
      return doc;
  };
}

/**
 * Generates deeply nested documents
 */
function nestedGenerator(big) {
    var strings = [];
    for (var i = 0; i < 26; i++) {
        strings.push(String.fromCharCode(97+i));
    }
    var i = 0;
    var levelSize = big ? 26 : 13;
    return function() {
      doc = {};
      for (var j = 0; j < levelSize; j++) {
            doc[strings[j]] = {};
            for (var k = 0; k < levelSize; k++) {
                doc[strings[j]][strings[k]] = {};
                for (var l = 0; l < levelSize; l++) {
                    doc[strings[j]][strings[k]][strings[l]] = {};
                    for (var m = 0; m < levelSize; m++) {
                        doc[strings[j]][strings[k]][strings[l]][strings[m]]
                            = i + j + k + l + m;
                    }
                }
            }
        }
      i++;
      return doc;
  };
}


// Functions to perform queries. Separated into functions so we can profile the calls. 

function queryEquals(t) {
  return t.find({x:1});
}

function whereDoubleEquals(t) {
  return t.find({$where: function() {return this.x == 1;}});
}

function whereTripleEquals(t) {
  return t.find({$where: function() {return this.x === 1;}});
}



// Queries that can be written in query language and using $where


generateDocs(1000, increasingXGenerator())(t);

var queryLanguageCursorEquals = queryEquals(t);
var whereCursorDoubleEquals = whereDoubleEquals(t);
var whereCursorTripleEquals = whereTripleEquals(t);

while (queryLanguageCursorEquals.hasNext()) {
  assert(whereCursorDoubleEquals.hasNext());
  assert(whereCursorTripleEquals.hasNext());
  var x = queryLanguageCursorEquals.next();
  var y = whereCursorDoubleEquals.next();
  var z = whereCursorTripleEquals.next();
  assert( x.x == y.x, "double equals doesn't match query language");
  assert( x.x == z.x, "triple equals doesn't match query language");
}

/** Simple Nested **/
generateDocs(13, nestedGenerator())(t);
var queryCursor = t.find( {'d.c.b.a' : 7} );
var whereCursor = t.find( { $where: 'return this.d.c.b.a == 7;' } );
while (queryCursor.hasNext()) {
    assert( queryCursor.next().a.b.c.d == whereCursor.next().a.b.c.d, "SimpleNested" );
}

// Queries that require the use of $where

generateDocs(200, tupleGenerator(200))(t);

var compareFields = t.find({$where: function() {return this.x == this.y; }});
while (compareFields.hasNext()) {
  var doc = compareFields.next();
  assert(doc.x == doc.y, "compareFields error: this.x != this.y")
}

compareFields = t.find({$where: function() {return this.x > this.y; }});
while (compareFields.hasNext()) {
  var doc = compareFields.next();
  assert(doc.x > doc.y, "compareFields error: this.x != this.y")
}

compareFields = t.find({$where: function() {return this.x >= this.y; }});
while (compareFields.hasNext()) {
  var doc = compareFields.next();
  assert(doc.x >= doc.y, "compareFields error: this.x !>= this.y")
}

compareFields = t.find({$where: function() {return this.x < this.y; }});
while (compareFields.hasNext()) {
  var doc = compareFields.next();
  assert(doc.x < doc.y, "compareFields error: this.x !< this.y")
}

compareFields = t.find({$where: function() {return this.x <= this.y; }});
while (compareFields.hasNext()) {
  var doc = compareFields.next();
  assert(doc.x <= doc.y, "compareFields error: this.x !<= this.y")
}


var mixed = t.find({$or : [{x: 2}, {$where: function() {return (this.y == 3);}}]});
while (compareFields.hasNext()) {
  var doc = compareFields.next();
  assert((doc.x == 2) || (doc.y == 3));
}

generateDocs(10, nestedGenerator(true))(t);

var complexNested = t.find({'$where': function() { return this.d.c.b.a === this.a.b.c.d; }});
while (complexNested.hasNext()) {
  var doc = complexNested.next();
  assert(doc.d.c.b.a == doc.a.b.c.d);
}


// Queries to experiment with document size

/*
 * Setup: Creates a collection of 10 documents, each with 4 nested levels of 26 fields
 * Test: Find document through match of a deeply nested field using $where
 */
tests.push({name: "Where.ReallyBigNestedComparison.Where",
            tags: ['query','where'],
            pre: generateDocs(10, nestedGenerator(true)),
            ops: [
              {op: "find", query: {'$where': function() { return this.a.b.c.d == 1; }}}
            ]
            } );

/*
 * Setup: Creates a collection of 10 documents, each with 4 nested levels of 26 fields
 * Test: Find document through match of a deeply nested field using query language
 */
tests.push({name: "Where.ReallyBigNestedComparison.QueryLanguage",
            tags: ['query','compare'],
            pre: generateDocs(10, nestedGenerator(true)),
            ops: [
              {op: "find", query: { 'a.b.c.d' : 1 }}
            ]
            });

lookAtDocumentMetrics = false;

if ( lookAtDocumentMetrics ) {
    // ignore mongos
    nscannedEnd = db.serverStatus().metrics.queryExecutor.scanned
    assert.lte( nscannedStart + 1, nscannedEnd );
}
