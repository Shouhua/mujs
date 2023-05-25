console.log("before setTimeout")
var id2 = setInterval(function(){console.log("interval callback2")}, 1000)
console.log("after interval")
var id = setTimeout(function(i, i1, i2, i3){
	clearTimeout(i)
	console.log("timeout callback", i1, i2, i3)
}, 2000, id2, 23, 24, 25)
console.log("after timeout")

console.log('before')
setTimeout(function(){
	console.log('timeout1')
	setTimeout(function(){
		console.log('timeout2')
		setTimeout(function(){
			console.log('timeout3')
		}, 10)
		setTimeout(function(){
			console.log('timeout4')
		}, 10)
	}, 10)
}, 10)
console.log('after')