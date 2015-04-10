'use strict';

// Setting up route
angular.module('core').config(['$stateProvider', '$urlRouterProvider',
	function($stateProvider, $urlRouterProvider) {
		// Redirect to welcome view when route not found
		$urlRouterProvider.otherwise('/');

		// Welcome state routing
		$stateProvider.
		state('home', {
			url: '/',
			templateUrl: 'modules/core/views/welcome.client.view.html'
		}).
		// Bigwhoop route
		state('bigwhoop', {
			url: '/bigwhoop',
			templateUrl: 'modules/core/views/home.client.view.html'
		});
	}
]);