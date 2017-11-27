//ram-Ctrl.js
accApp.controller('ramController', ['$scope', 'socket' ,'$timeout', '$compile', '$http', '$uibModal',function($scope, socket, $timeout, $compile, $http, $uibModal)
{

    $scope.myacc_collection = [];

    var getMyRamObj= function(acc_id)
    {
    	var obj = new Object();
    	for(var i = 0; i < $scope.myacc_collection.length; i++)
    	{
    		if(acc_id == $scope.myacc_collection[i].acc_id)
    		{	
    			obj = $scope.myacc_collection[i];
                return obj;
    		}

    	}
    }

    $scope.sendResetModule = function(my_acc_id)
    {
      socket.emit('resetModule', {acc_id: my_acc_id, command: "resetModule"});
    }

    $scope.sendInitModule = function(my_acc_id)
    {
      socket.emit('initModule', {acc_id: my_acc_id, command: "resetModule"});
    }

    socket.on('acc_ram', function(data) 
    {
      for(var i=0; i < $scope.myacc_collection.length; i++)
      {
       if($scope.myacc_collection[i].acc_id == data.acc_ram.acc_id)
       {
          return;//==========> I already have this one
       }
      }

      var obj = new Object();
      obj.acc_id = data.acc_ram.acc_id;
      obj.x = 0; obj.y=0; obj.z =0;

      var speed = 20;//ms per pixel
      var smoothie = new SmoothieChart({ millisPerPixel:speed,               
                                          timestampFormatter:SmoothieChart.timeFormatter });      
      obj.axeX = new TimeSeries();
      obj.axeY = new TimeSeries();
      obj.axeZ = new TimeSeries();
      smoothie.addTimeSeries(obj.axeX, {  strokeStyle:'green',     lineWidth:2}  );
      smoothie.addTimeSeries(obj.axeY, {  strokeStyle:'#66ff33',   lineWidth:2}  );
      smoothie.addTimeSeries(obj.axeZ, {  strokeStyle:'blue',      lineWidth:2}  );      
      obj.smoothie = smoothie;

      $timeout(function() {
        obj.smoothie.streamTo(document.getElementById("canvas_"+obj.acc_id), 1000 /*delay*/);
      }, 2000);
      
      $scope.myacc_collection.push(obj);    
      return; //===========>
    });


         
    socket.on('welcome', function(data) 
    {
          //console.log("welcome recived Send atime ")
          //$('#messages').append('<li>' + data.message + '</li>');
          socket.emit('atime', {data: 'foo!'});
          //console.log(data);
    });


    socket.on('acc_data', function(data) 
    {
     console.log(data.acc_data); 
     //var currentDate = new Date().getTime();
     
     for(var i=0; i < $scope.myacc_collection.length; i++)
     {        
        if(data.acc_data.device_name == $scope.myacc_collection[i].acc_id)
        {
          $scope.myacc_collection[i].x = Number(data.acc_data.x);
          $scope.myacc_collection[i].y = Number(data.acc_data.y);
          $scope.myacc_collection[i].z = Number(data.acc_data.z);

          var currentDate = new Date().getTime();
          $scope.myacc_collection[i].axeX.append(currentDate, Number(data.acc_data.x));
          $scope.myacc_collection[i].axeY.append(currentDate, Number(data.acc_data.y));
          $scope.myacc_collection[i].axeZ.append(currentDate, Number(data.acc_data.z));

           return; 
        }
      }
    });


    socket.on('error', function() { console.error(arguments) });
    socket.on('message', function() { console.log("message-----");console.log(arguments) });

}]);
