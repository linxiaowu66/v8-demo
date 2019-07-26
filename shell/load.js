function sum(){
	var s = 0;
	for(var i = 0; i < arguments.length; i++){
	    print(arguments[i])
		s += arguments[i];
	}
	return s;
}

function avg(){
	var args = arguments;
	var count = args.length;
	var sum = 0;
	for(var i = 0; i < count; i++){
		sum += args[i];
	}
	print(version)
	return sum/count;
}