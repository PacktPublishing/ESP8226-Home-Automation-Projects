//app.js
var accApp = angular.module('accApp', [ 'ui.router', 'ngSanitize', 'ui.bootstrap']);

accApp.directive('stringToNumber', function() {
  return {
    require: 'ngModel',
    link: function(scope, element, attrs, ngModel) {
      ngModel.$parsers.push(function(value) {
        return '' + value;
      });
      ngModel.$formatters.push(function(value) {
        return parseFloat(value);
      });
    }
  };
});

accApp.config(function($stateProvider, $urlRouterProvider) 
{
    
    $urlRouterProvider.otherwise('/home');
    
    $stateProvider

    // HOME STATES AND NESTED VIEWS ========================================
    .state('home', {
        url: '/home',
        templateUrl: 'views/home.html',
    })

}); // closes $routerApp.config()