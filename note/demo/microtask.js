console.log('before')
setTimeout(function(){
	console.log(1)
}, 10)
queueMicrotask(function(){
	console.log('microtask1')
	setTimeout(function(){
		queueMicrotask(function() {
			console.log('queue micro task...')
		})
		console.log('timeout1')
	}, 10)
})
console.log('middle')
setTimeout(function(){
	console.log(0)
}, 11)
queueMicrotask(function(){
	console.log('microtask2')
	setTimeout(function(){
		console.log('timeout2')
	}, 10)
})
console.log('after')
setTimeout(function(){
	console.log(2)
}, 10)