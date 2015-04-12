'use strict';

/**
 * Module dependencies.
 */
var _ = require('lodash');

/**
 * Extend bigwhoop's controller
 */
module.exports = _.extend(
	require('./bigwhoop/bigwhoop.dataflow')
);