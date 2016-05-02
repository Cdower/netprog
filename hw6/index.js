var express = require('express');//express
var app = express();

app.listen(3000, function () {
  console.log('Example app listening on port 3000!');
});

app.get('/', function (req, res) {
  res.sendFile( __dirname + '/hw6.html');
});
