var TargetManager = function TargetManager(face) {
    this.target = '';
    this.CD = document.getElementById('name').value;
    this.face_ = face;
};

TargetManager.prototype.run = function() {
    this.sendRequest();
    var self = this;
    return setInterval(function() { self.sendRequest(); }, REQUEST_REPLAYINTERBAL_MILLISECONDS);
};

TargetManager.prototype.setTarget = function(target) {
    this.target = target;
    return true;
};

TargetManager.prototype.setName = function(name) {
    this.CD = name;
    return true;
};

TargetManager.prototype.getTarget = function() {
    return this.target;
};

TargetManager.prototype.sendRequest = function() {
    var targetJson = {};
    targetJson.target = this.target;
    if(this.target != '') {
        if(isFinding == false){
            isFinding = true;
            this.sendInterest(this.CD, targetJson);
        }
    }
};

TargetManager.prototype.sendInterest = function(name, parameters) {
    var T0 = new Date();
    var interest = new Interest(new Name(name));
    //interest.setParameters(JSON.stringify(parameters));
    interest.setInterestLifetimeMilliseconds(INTEREST_LIFETIME_MILLISECONDS);
    interest.setMustBeFresh(true);

    printLine2("Sent request of name: " + name + ", target: " + parameters.target + ", parameter: " + JSON.stringify(parameters));

    this.face_.expressInterest(interest, function(interest, content) { onData(interest, content, T0); }, onTimeout);
};
