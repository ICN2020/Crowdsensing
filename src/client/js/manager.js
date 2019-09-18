function printStatus(line) {
    var date = new Date() ;
    timestamp = date.toDateString();
    timestamp += "  ";
    timestamp += date.toLocaleTimeString();

    var receiveLog = document.getElementById('receiveLog');
    receiveLog.insertAdjacentHTML('afterbegin',"[" + timestamp + "]" + "<br>" + line + "<br>" + "<br>");
    receiveLog.scrollTop = 0;
}

function printLine2(line) {
    var date = new Date() ;
    timestamp = date.toDateString();
    timestamp += "  ";
    timestamp += date.toLocaleTimeString();

    var sendLog = document.getElementById('sendLog');
    sendLog.insertAdjacentHTML('afterbegin',"[" + timestamp + "]" + "<br>" + line + "<br>" + "<br>");
    sendLog.scrollTop = 0;
}

function draw() {
    $('#cells').find('tr').each(function(i, elemTr) { 
        $(elemTr).children().each(function(j, elemTd) { 
            $(elemTd).removeClass(); 
            switch (cellsArray[i][j]) {
            case 1:
                $(elemTd).addClass("found"); 
                break;
            case 2:
                $(elemTd).addClass("notFound"); 
                break;
            default:
                $(elemTd).addClass("default");
            }
        });
    });
}

function convertZordertoXY(zorder) {
    var x, y;
    var zorderString = String(zorder);
    var length = zorder.toString().length;
    var zorderArray = new Array();
    var zorderBinArray = new Array();
    var xArray = new Array();
    var yArray = new Array();

    console.log(zorder);
    console.log(length);
    
    x = 0;
    y = 0;

    for(var i=0; i < length; i++){
        zorderArray[i] = parseInt(zorderString.charAt(i), 10);
        zorderBinArray[i] = ( '00' + zorderArray[i].toString(2) ).slice( -2 );

        if(zorderBinArray[i].charAt(1) != 1){
            xArray[i] = 0;
        }else{
            xArray[i] = 1;
        }

        if(zorderBinArray[i].charAt(0) != 1){
            yArray[i] = 0;
        }else{
            yArray[i] = 1;
        }
    }
    for(var i = 0; i < length; i++){
        x = x * 2;
        y = y * 2;
        x += parseInt(xArray[i], 10);
        y += parseInt(yArray[i], 10);
    }
    return [x, y];
}

function markingXY(x, y, isFound) {
    if(isFound) {
        cellsArray[y][x] = 1;
    }else{
        cellsArray[y][x] = 2;
    }
    
    draw();
}

function mapFlash() {
    if($('.gwd-img-111z').hasClass('back')){
        $('.gwd-img-111z').removeClass('back');
    }else{
        $('.gwd-img-111z').addClass('back');
    }
}

function markingFlash() {
    if($('td').hasClass('front')){
        $('td').removeClass('front');
    }else{
        $('td').addClass('front');
    }
}

var Echo = function Echo(keyChain, face) {
    this.keyChain = keyChain;
    this.face = face;
};

Echo.prototype.onRegisterFailed = function(prefix) {
    //printLine("Register failed for prefix " + prefix.toUri());
};

Echo.prototype.onInterest = function(prefix, interest, face, interestFilterId, filter) {
    console.log("debug of onInterest.");
    // Make and sign a Data packet.
    var data = new Data(interest.getName());
    var content = "Echo " + interest.getName().toUri();
    data.setContent(content);
    // Use KeyChain.sign with the onComplete callback so that it works with Crypto.subtle.
    this.keyChain.sign(data, function() {
        try {
            //printLine("Sent content " + content);
            face.putData(data);
        } catch (e) {
            //printLine(e.toString());
        }
    });
};

function onTimeout(interest) {
    printLine2("Request Timeout");
    isFinding = false;
};

function onData(interest, content, T0) {
    var T1      = new Date();
    const name  = content.getName().toUri().split("/").slice(0,-2).join("/");
    var payload = DataUtils.toString(content.getContent().buf());

    var stringContentArray = payload.trim().split(/\r\n|\r|\n/);
    var resultJsonObject = [];
    for(var i = 0; i < stringContentArray.length; i++) {
        console.log("this is debug for split" + i);
        resultJsonObject.push(JSON.parse(stringContentArray[i]));
    }

    console.log("Data packet received.");
    printLine2("Payload: " + payload);

    for(var i = 0; i < resultJsonObject.length; ++i) {
        if(resultJsonObject[i].isFound){
            printStatus("[" + resultJsonObject[i].target + "]" + "Target Found!");
            var targetHTMLList = document.getElementById(resultJsonObject[i].target);
            targetHTMLList.innerHTML += "[" + resultJsonObject[i].target + "]" + ": Found";
            targetHTMLList.innerHTML += "<br>" + "Location: " + Number(resultJsonObject[i].location);
            targetHTMLList.innerHTML += "<br>" + "Time: " + resultJsonObject[i].time + "<br><br>";
        } else {
            printStatus("[" + resultJsonObject[i].target + "]" + "Target Not Found");
            var targetHTMLList = document.getElementById(resultJsonObject[i].target);
            targetHTMLList.innerHTML += "[" + resultJsonObject[i].target + "]" + ": Not Found";
            targetHTMLList.innerHTML += "<br>" + "Location: " + Number(resultJsonObject[i].location);
            targetHTMLList.innerHTML += "<br>" + "Time: " + resultJsonObject[i].time + "<br><br>";
        }
        var cell = convertZordertoXY(parseInt(resultJsonObject[i].location, 10) - 30000);
        markingXY(cell[0], cell[1], resultJsonObject[i].isFound);
    }

    isFinding = false;
}
