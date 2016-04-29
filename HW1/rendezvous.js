/*
 * rendezvous.js
 * Andrew Armenia
 * 3-1-2015
 * 
 * Rendezvous server for Network Programming Homework 1.
 * Listens on udp port 12345.
 * 
 * Run this server as: node rendezvous.js.
 * 
 * DO NOT USE NODE.JS TO WRITE YOUR SOLUTION
 * YOUR SUBMISSION MUST BE IN C OR C++!
 */

var dgram = require('dgram');

// create an object to store peer addresses
var registry = { };
registry['echo'] = "127.0.0.1 12346";

// create a socket and bind to port 12345
var s = dgram.createSocket('udp6');
s.bind(12345);

// handle incoming messages
s.on('message', function(msg, rinfo) {
        console.log('server got: ' + msg + ' from ' + rinfo.address + ':' + rinfo.port);
	var rhost = rinfo.address;
	var rport = rinfo.port;
	var raddr = rhost + " " + rport;
	
	// parse the request
	var match = /^([A-Z_]+) ([A-Za-z0-9_]+)/.exec(msg);
	if (match) {
		// split into the verb and the peer name
		var verb = match[1];
		var name = match[2];

		if (verb == "REGISTER") {
			// record peer info in registry
			console.log(
				"Registered node %s at address %s", 
				name, raddr
			);
			registry[name] = raddr;
		} else if (verb == "GET_ADDR") {
			// send back the address from the registry
			var result = registry[name];
			if (result) {
				console.log(
					"Address of %s is %s",
					name, result
				);
			} else {
				console.log("unknown peer %s", name);
				result = "NOT FOUND";
			}
			
			var packet = new Buffer(result);
			s.send(packet, 0, packet.length, rport, rhost);
		} else {
			// invalid verb
			console.log("Verb %s invalid", verb);
		}
	}
});

var dgram = require('dgram');
var s2 = dgram.createSocket('udp6');
s2.bind(12346);
s2.on('message', function(msg, rinfo) {
	var packet = new Buffer(msg);
	var rhost = rinfo.address;
	var rport = rinfo.port;
	s2.send(packet, 0, packet.length, rport, rhost);
});
console.log("hello world");
