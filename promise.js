function Promise(handler) {
	this._status = 'pending'
	this._value = null
	this._onFulfilledCallbacks = []
	this._onRejectedCallbacks = []
	try {
		handler(this._resolve.bind(this), this._reject.bind(this))	
	} catch (err) {
		this._reject.bind(this, err)
	}
}
Promise.prototype._resolve = function(_value) {
	var self = this;
	if(self._status === 'pending') {
		self._status = 'fulfilled'
		self._value = _value
		self._onFulfilledCallbacks.forEach(function(fn){
			fn(_value)
		})
	}
}
Promise.prototype._reject = function(_value) {
	var self = this;
	if(self._status === 'pending') {
		self._status = 'rejected'
		self._value = _value
		self._onRejectedCallbacks.forEach(function(fn){
			fn(_value)
		})
	}
}
Promise.prototype.then = function(onFulfilled, onRejected) {
	var self = this;
	return new Promise(function(resolve, reject) {
		if(self._status === 'pending') { // 异步支持，因为setTimeout还没有返回，进来的时候还是pending状态
			self._onFulfilledCallbacks.push(function(){
				try {
					resolve(onFulfilled(self._value))
				} catch(err) {
					reject(err)
				}
			})
			self._onRejectedCallbacks.push(function() {
				try {
					resolve(onRejected(self._value))
				} catch(err) {
					reject(err)
				}
			})
		}
		if(self._status === 'fulfilled') {
			queueMicrotask(function(){
				try {
					// TODO self生效，使用self值为undefined
					resolve(onFulfilled(self._value))
				} catch(err) {
					reject(err)
				}
			})
		}
		if(self._status === 'rejected') {
			queueMicrotask(function(){
				try {
					resolve(onRejected(self._value))
				} catch(err) {
					reject(err)
				}
				
			})
		}
	}) 
}
Promise.prototype.catch = function(onRejected) {
	var self = this
	return self.then(undefined, onRejected)
}
Promise.prototype.finally = function(cb) {
	var self = this
	return self.then(
		function(value){ Promise.resolve(cb()).then(function(){return value})},
		function(reason) {Promise.resolve(cb()).then(function() {throw reason })}
	)
}
Promise.prototype.done = function(onFulfilled, onRejected) {
	var self = this
	self.then(onFulfilled, onRejected)
		.catch(function(reason) {
			throw(reason)
		})
}
Promise.resolve = function(value) {
	if(value instanceof Promise) return value
	return new Promise(function(resolve) {resolve(value)})
}
Promise.reject = function(value) {
	if(value instanceof Promise) return value
	return new Promise(function(resolve, reject) {reject(value)})
}
Promise.all = function(promises) {
	return new Promise(function(resolve, reject) {
		var result = []
		var count = 0
		for(var i = 0; i < promises.length; i++) {
			Promise.resolve(promises[i]).then(function(res) {
				result.push(res)
				count++
				if(count === promises.length) {
					resolve(result)
				}
			}, function(err) {
				reject(err)
			})
		}
	})
}
Promise.reace = function(promises) {
	return new Promise(function(resolve, reject) {
		for(var i = 0; i < promises.length; i++) {
			Promise.resolve(promises[i]).then(function(res) {
				resolve(res)
			}, function(err) {
				reject(err)
			})
		}
	})
}