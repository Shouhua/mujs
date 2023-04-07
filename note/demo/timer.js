console.log("before setTimeout")
var id2 = setInterval(function(){console.log("interval callback2")}, 1000)
console.log("after interval")
var id = setTimeout(function(i){
	clearTimeout(i)
	console.log("timeout callback")
}, 3000, id2)
console.log("after timeout")