'use strict';


angular.module('core').controller('HomeController', ['$scope','$http', 'Authentication',
    function($scope, $http, Authentication) {
        // This provides Authentication context.
        $scope.authentication = Authentication;

        // map center
        var myLatlng = new google.maps.LatLng(52.4935, 7.629);
        // map options,
        var myOptions = {
            zoom: 3,
            center: myLatlng
        };
        // standard map
        $scope.map = new google.maps.Map(document.getElementById("map-canvas"), myOptions);
	        // heatmap layer
	        $scope.heatmap = new HeatmapOverlay($scope.map, {
	            // radius should be small ONLY if scaleRadius is true (or small radius is intended)
	            "radius": 4,
	            "maxOpacity": 1,
	            // scales the radius based on map zoom
	            "scaleRadius": true,
	            // if set to false the heatmap uses the global maximum for colorization
	            // if activated: uses the data maximum within the current map boundaries 
	            //   (there will always be a red spot with useLocalExtremas true)
	            "useLocalExtrema": true,
	            // which field name in your data represents the latitude - default "lat"
	            latField: 'lat',
	            // which field name in your data represents the longitude - default "lng"
	            lngField: 'lon',
	            // which field name in your data represents the data value - default "value"
	            valueField: 'max_amplitude'
	        });
		    
        $http.get('modules/core/json/bigwhoop.json').success(function(bigwhoop) {
		    console.warn(bigwhoop.data.dataset);
		    var testData = {
	            max: 100000,
	            data: bigwhoop.data.dataset.analyze_full_spectrum_basic
	            //bigwhoop.data.dataset
	            // [{
	            //     lat: 24.6408,
	            //     lng: 46.7728,
	            //     count: 14
	            // }]
	        };
	        $scope.heatmap.setData(testData);
		});
        
    }
]);