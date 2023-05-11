var xhr = new XMLHttpRequest()
// var url = 'https://jsonplaceholder.typicode.com/todos/1'
var url = 'http://192.168.126.128:3000/echo'
// var url = 'https://www.baidu.com'
// xhr.open('GET', url)
console.log('Begin request...')
xhr.open('POST', url, false)
// xhr.open('GET', url)
xhr.setRequestHeader("content-type", "application/x-www-form-urlencoded")
// xhr.setRequestHeader("content-type", "application/json")
// xhr.timeout = 1000
// xhr.responseType = 'json'
xhr.ontimeout = function() {
	console.log('timeout trigger')
}
xhr.onload = function() {
	// console.log('i: ', i)
	// console.log('xhr.toString(): ', xhr.toString())
	// console.log('xhr.timeout: ', xhr.timeout)
	// console.log('xhr.responseType: ', xhr.responseType)
	console.log('xhr.getResponseHeader("x-custom"):', xhr.getResponseHeader('x-custom'))
	console.log('xhr.response: ', JSON.stringify(xhr.response))
	// console.log('xhr.response: ', JSON.stringify(xhr.response))
	// console.log('xhr.response: ', xhr.response)
}
// xhr.send(JSON.stringify({
// 	name: 'admin',
// 	age: 38
// }));
xhr.send("foo=bar&lorem=ipsum")
console.log("End request...")