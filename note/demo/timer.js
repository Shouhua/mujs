// console.log("before setTimeout")
// var id2 = setInterval(function(){console.log("interval callback2")}, 1000)
// console.log("after interval")
// var id = setTimeout(function(i){
// 	clearTimeout(i)
// 	console.log("timeout callback")
// }, 3000, id2)
// console.log("after timeout")
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