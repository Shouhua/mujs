var i = 1000
var index = setTimeout(function() {
	console.log('timeout: 1000ms')
}, i);
setInterval(function() {
	console.log('interval: 1000ms')
}, 1000)
var xhr = new XMLHttpRequest()
// var url = 'https://jsonplaceholder.typicode.com/todos/1'
var url = 'http://192.168.126.128:3000/api'
// var url = 'https://www.baidu.com'
xhr.open('GET', url, true)
// xhr.timeout = 1000
xhr.responseType = 'json'
xhr.onload = function() {
	console.log('i: ', i)
	console.log('xhr.toString(): ', xhr.toString())
	console.log('xhr.timeout: ', xhr.timeout)
	console.log('xhr.responseType: ', xhr.responseType)
	res = xhr.response
	console.log(res.name)
	console.log(res.age)
	// console.log('xhr.response: ', JSON.stringify(xhr.response))
	// console.log('xhr.response: ', xhr.response)
}
xhr.send();