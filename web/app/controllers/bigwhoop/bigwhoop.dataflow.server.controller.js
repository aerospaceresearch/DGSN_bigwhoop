'use strict';

/**
 * Module dependencies.
 */
var _ = require('lodash'),
	errorHandler = require('../errors');

/**
 * Recieve data and try to propagate
 */
exports.dataflow = function(req, res) {
	var bigWhoopData = req.body;
	console.warn(bigWhoopData);
	if(bigWhoopData){


		res.status(200).send({
			message: 'Thanks for your submission'
		});
	}
	else{
		res.status(400).send({
			message: 'Can\'t get your data at the moment'
		});
	}
	
};