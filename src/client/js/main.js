var face;
var targetManager;
var RunID;
var isFinding;

function main() {
    draw();
    // setInterval(function() { mapFlash(); }, 2000);
    // setInterval(function() { markingFlash(); },1000);

    var CD = document.getElementById('name').value;
    var host = "localhost";

    // Connect to the forwarder with a WebSocket.
    face = new Face({host: host});
    //face = new Face({host: "udp://192.168.10.97"});
    //face = new Face();

    // For now, when setting face.setCommandSigningInfo, use a key chain with
    //   a default private key instead of the system default key chain. This
    //   is OK for now because NFD is configured to skip verification, so it
    //   ignores the system default key chain.
    // On a platform which supports it, it would be better to use the default
    //   KeyChain constructor.
    var keyChain = new KeyChain("pib-memory:", "tpm-memory:");
    keyChain.importSafeBag(new SafeBag
                           (new Name("/testname/KEY/123"),
                            new Blob(DEFAULT_RSA_PRIVATE_KEY_DER, false),
                            new Blob(DEFAULT_RSA_PUBLIC_KEY_DER, false)));

    face.setCommandSigningInfo(keyChain, keyChain.getDefaultCertificateName());

    var echo = new Echo(keyChain, face);
    var prefix = new Name(CD);

    targetManager = new TargetManager(face);

    isFinding = false;
    Run();
}

function RegisterTarget(){
    var target = document.getElementById('target').value;
    var name = document.getElementById('name').value;

    if(targetManager.setTarget(target)) {
        console.log("target registered");
    }

    if(targetManager.setName(name)) {
        console.log("name registered");
    }

    var targetList = document.getElementById('targetList');
    targetList.innerHTML += '<p id="' + target + '">' + "\n[" + target + "] Finding..." + '</p>';
}


function Run(){
    RunID = targetManager.run();
    console.log("system started");
}

function Stop(){
    clearInterval(RunID);
    console.log("system stopped");
}
