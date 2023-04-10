var i = 1000
var index = setInterval(function() {
	console.log("interval: 1000ms")
}, i);
var xhr = new XMLHttpRequest()
var url = "https://jsonplaceholder.typicode.com/todos/1"
xhr.open("GET", url, true)
xhr.onload(function() {
	console.log("hello, world ", i)
	console.log(xhr.response())
})
xhr.send();