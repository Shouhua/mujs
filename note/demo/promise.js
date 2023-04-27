console.log('promise begin')
var p1 = new Promise(function(resolve, reject) {
	setTimeout(function() {
		resolve(1)	
	}, 1000)
})
p1.then(function(res){
	console.log(res)
	return res+1
}).then(function(res) {
	console.log(res)
})
var promise2 = Promise.resolve(3);
var promise3 = 42;
var promise4 = new Promise(function(resolve, reject) {
		setTimeout(resolve, 100, 'foo');
});

var newPromise = Promise.all([promise2, promise3, promise4]).then(function(values) {
	console.log(values)
});
console.log('promise end')