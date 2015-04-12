'use strict';

module.exports = function(app) {
	// Root routing
	var bigwhoop = require('../../app/controllers/bigwhoop');
	app.route('/bigwhoop/data').post(bigwhoop.dataflow);
};