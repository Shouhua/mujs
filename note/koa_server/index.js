const fs = require('fs')
const path = require('path')
const Koa = require('koa')
const bodyParser = require('koa-bodyparser')

const app = new Koa()
app.use(bodyParser())

app.use(async ctx=> {
    if(ctx.url === '/') {
        const data = fs.readFileSync(path.resolve(__dirname, 'index.html'))
        ctx.type = 'text/html'
        ctx.body = data
        return;
    }
    if(ctx.url === '/echo') {
        console.log(ctx.get('content-type'))
        console.log(ctx.request.body)
        ctx.set("x-custom", "hello")
        ctx.append("x-custom", "world")
        ctx.append("x-custom", "!")
        ctx.type = 'application/json'
        ctx.body = ctx.request.body;
        return;
    }
    if(ctx.url === '/api') {
        // await new Promise(resolve => setTimeout(resolve, 3000));
        ctx.type = 'application/json'
        console.log("Accept: ", ctx.get("Accept"));
        ctx.set("x-custom", "hello")
        ctx.append("x-custom", "world")
        ctx.append("x-custom", "!")

        ctx.body = JSON.stringify({
            "name": "Lebron James",
            "age": 38
        })
        return
    }
    ctx.body = 'Hello, world!'
})

app.listen(3000)
