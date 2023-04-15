console.log('before')
queueMicrotask(function(){
	console.log('microtask1')
	setTimeout(function(){
		console.log('timeout1')
	}, 10)
})
console.log('middle')
queueMicrotask(function(){
	console.log('microtask2')
	var i = setTimeout(function(){
		console.log('timeout2')
	}, 10)
})
console.log('after')
setTimeout(function(){
	console.log(1)
}, 10)
setTimeout(function(){
	console.log(2)
}, 100)