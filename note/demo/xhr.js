var i = 3000
var index = setTimeout(function(str) {
	console.log('timeout: 1000ms', str)
	clearInterval(i2)
}, i, 'helo');
var i2 = setInterval(function() {
	console.log('interval: 1000ms')
}, 1000)
var xhr = new XMLHttpRequest()
var url = 'https://jsonplaceholder.typicode.com/todos/1'
// var url = 'http://192.168.126.128:3000/api'
// var url = 'https://www.baidu.com'
// xhr.open('GET', url)
console.log('Begin request...')
xhr.open('GET', url, false)
// xhr.timeout = 1000
xhr.responseType = 'json'
xhr.ontimeout = function() {
	console.log('timeout trigger')
}
xhr.onload = function() {
	// console.log('i: ', i)
	// console.log('xhr.toString(): ', xhr.toString())
	// console.log('xhr.timeout: ', xhr.timeout)
	// console.log('xhr.responseType: ', xhr.responseType)
	console.log('xhr.response: ', JSON.stringify(xhr.response))
	// console.log('xhr.response: ', JSON.stringify(xhr.response))
	// console.log('xhr.response: ', xhr.response)
}
xhr.send();
console.log("End request...")