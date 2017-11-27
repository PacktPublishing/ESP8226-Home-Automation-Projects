//server.js

var port     = process.env.PORT || 1234;
var fs           = require('fs');                     // for using file system
var logger       = require('morgan');       // log requests to the console (express4)
var cookieParser = require('cookie-parser');
var bodyParser   = require('body-parser');   // pull information from HTML POST (express4)
var session      = require('express-session');
var http         = require('http');
var express      = require('express'),
    app = module.exports.app = express();
var server = http.createServer(app);
var io = require('socket.io').listen(server);  //pass a http.Server instance

app.use(logger('dev')); // log every request to the console
app.use(cookieParser()); // read cookies (needed for auth)
app.use(bodyParser.json()); // get information from html forms
app.use(bodyParser.urlencoded({ extended: true })); //true or false ????

app.use(express.static(__dirname + '/public'));
app.set('views', __dirname + '/public');
app.engine('html', require('ejs').renderFile);
app.set('view engine', 'html');

app.use(function (req, res, next) {
  var ip = req.headers['x-forwarded-for'] || req.connection.remoteAddress;
  console.log('Client IP:', ip);
  next();
});

var webConnections = [];
var accConnections = [];


server.listen(1234);  //listen on port 80

console.log('The magic happens on port ' + port);


/*******************************
Function:
Description:
Parameters:
Return:
********************************/
function ParseJson(jsondata) {
    try {
        return JSON.parse(jsondata);
    } catch (error) {
        return null;
    }
}

/*******************************
Function:
Description:
Parameters:
Return:
********************************/
function printWEB()
{
 console.log("=======================================================");
 console.log("|ID| Web Socket ID                                    |");
 console.log("=======================================================");
 for(var i=0 ; i < webConnections.length; i++)
 {
  
  console.log("| "+ i + "| " + webConnections[i].socket_id );
  //console.log("-------------------------------------------------------");
  //console.log(webConnections[i].socket_id);
 }
 console.log("=======================================================");
}

/*******************************
Function:
Description:
Parameters:
Return:
********************************/
function printACC()
{
 console.log("========================================");
 console.log("|ID| ACC ID     | Socket ID            |");
 console.log("========================================");
 for(var i=0 ; i < accConnections.length; i++)
 {
  
  console.log("| "+ i + "| " + accConnections[i].acc_id + " | " + accConnections[i].socket_id + " | " );
  //console.log("-------------------------------------------------------");
 }
 console.log("=======================================================");
}

/*******************************
Function:
Description:
Parameters:
Return:
********************************/
//Just in case
function deleteRAM(socket_id)
{
  for(var i=0; i < accConnections.length; i++)
  {
    if(accConnections[i].socket_id == socket_id)
    {
      //remove this element from accConnections
    }
  }//end for
}

function addACCObject(obj)
{
  //console.log("addACCObject"); console.log(obj);
  for(var i=0; i < accConnections.length; i++)
  {
    if(accConnections[i].acc_id == obj.acc_id)
    {
       accConnections[i].socket_id = obj.socket_id;	   
       return; //=====>
    }    
  }
  //if we are here it means that this is a new request for RAM.
  accConnections.push(obj);
}

io.on('connection', function (socket) 
{
	console.log("Connected  socket id:" + socket.id); 

	if(socket.handshake.headers.origin != "ESP_ACC")
	{
       //possible web browser connection. ESP has Origin preset to ESP_RAM
        var web_obj = new Object();
        web_obj.socket_id = socket.id;
		    web_obj.socket = socket;
        webConnections.push(web_obj);
        console.log("SOCK: connection - new WEB connection!!!!!");
		
	    for(var webBrowsers = 0; webBrowsers < webConnections.length; webBrowsers++)
	    {
	       console.log("SOCK: RX connection => Send acc_ram with data");
         //resend the entire collection of known RAMs
  		   for(var j=0; j< accConnections.length; j++)
  		   {
  			   var ram_c = accConnections[j];
  			   var sessionID = webConnections[webBrowsers].socket;
  			   //var data = "{ acc_id: '"+ram_c.acc_id +"' , machine_id: '"+ram_c.machine_id+"'  }";
  			   var data = new Object();
  			   data.acc_id = ram_c.acc_id;			   
  			   console.log(data);
  	       sessionID.emit('acc_ram', { acc_ram: data });
  		   }
	    }		
      printWEB();
		  printACC();
	}
	else
	{
	    // here the connections from the ESP_RAM are received.
	    // add socket from ESP to the list of RAMs
	    //accConnections.push(socket);
	    console.log("SOCK: connection - new ACC connection");
	}

	socket.emit('welcome', { message: 'Connected !!!!' });
	
	
    /*********************************************************
     *     C O N N E C T I O N - rx connection msg from RAM
    **********************************************************/
	socket.on('connection', function (data) 
	{

	    var acc_ram = new Object();
	    acc_ram.socket_id = socket.id;
	    var data_json = ParseJson(JSON.stringify(data)); 
	    console.log(ParseJson(JSON.stringify(data))); 
	    acc_ram.acc_id = data_json.acc_id; //console.log(acc_ram.acc_id);
	    //acc_ram.machine_id = data_json.machine_id;
  		acc_ram.socket = socket;
      addACCObject(acc_ram);
	    printACC();
 
	    for(var webBrowsers = 0; webBrowsers < webConnections.length; webBrowsers++)
	    {
	       var sessionID = webConnections[webBrowsers].socket;
	       sessionID.emit('acc_ram', { acc_ram: data })		  
	    }

	});



  /*********************************************************
  *     J S O N - message from RAM. Send data to connected
  * web clients
  **********************************************************/
  socket.on('JSON', function (data) 
  {
    for(var webBrowsers = 0; webBrowsers < webConnections.length; webBrowsers++)
    {
  	  //send data to all connected browsers
  	  var sessionID = webConnections[webBrowsers].socket; 
      //console.log("emit acc_data to: " + data.device_name + " " + data.x + ", " + data.y + ", " + data.z);
  	  sessionID.emit('acc_data', { acc_data: data });
    }

  });
  
  socket.on('resetModule' , function (data)
  {
    //console.log("resetModule message received!")	
    for(var j=0; j< accConnections.length; j++)
   {
	  var acc_m = accConnections[j];
	  //console.log(acc_m.acc_id + " versus "); 
      if(acc_m.acc_id == data.acc_id)
	  {
		  //found RAM in my RAM list
		  var s = acc_m.socket; //Get the socket
		  s.emit("resetModule", {message: data} );
		  return; //======>
	  }
   }	
  });   

  socket.on('initModule' , function (data)
  {
    //console.log("initModule message received!")	
    for(var j=0; j< accConnections.length; j++)
   {
	  var ram_c = accConnections[j];
	  //console.log(ram_c.acc_id + " versus "); 
      if(ram_c.acc_id == data.acc_id)
	  {
		  //found RAM in my RAM list
		  var s = ram_c.socket; //Get the socket
		  s.emit("initModule", {message: data} );
		  return; //======>
	  }
   }	
  });   
  

  /*********************************************************
  *     D I S C O N N E C T
  **********************************************************/
  socket.on("disconnect" , function(reason) 
  {
	//console.log(socket.id);
	//console.log(reason);
	if(reason == "transport close")
	{
	 console.log("receive a disconnect")
	}

	//var index = webConnections.indexOf(socket.id); 
	var index = -1;
	for(var i=0; i<webConnections.length; i++)
	{
		if(webConnections[i].socket_id == socket.id)
		{
			index = i;
			break;
		}
	}
	
	//check if the disconnect is from a web page
	if (index != -1) {
		webConnections.splice(index, 1);
		console.info('WEB client gone (id=' + socket.id + ').');
		//reset the index back to its default value
		index = -1;			
	}
    //check if the disconnect is from a RAM module
	if (index != -1) 
	{
		accConnections.splice(index, 1);
        //inform all WEB browsers that a RAM is diconncted
	    for(var webBrowsers = 0; webBrowsers < webConnections.length; webBrowsers++)
        {
          var sessionID = webConnections[webBrowsers].socket;
          sessionID.emit('ram_disconnect', { ram_module: accConnections[index] });
        }		
		//console.info('RAM gone (id=' + socket.id + ').');
	}	
	printWEB();	
	printACC();
	
  });
});


