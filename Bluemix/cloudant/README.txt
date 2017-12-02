Cloudant DB is instantiated when the node is used in Node-RED. The following index should be added to the database that will be created.

{
 "type": "json",
 "def": {
  "fields": [
   {
    "time_stamp_nano": "asc"
   }
  ]
 }
}

The permissions will also need to be adjusted:
Under Permissions -> Unauthenticated connections -> _reader : check the box
