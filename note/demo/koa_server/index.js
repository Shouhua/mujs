const fs = require('fs')
const path = require('path')
const Koa = require('koa')

const app = new Koa()

app.use(async ctx=> {
    if(ctx.url === '/') {
        const data = fs.readFileSync(path.resolve(__dirname, 'index.html'))
        ctx.type = 'text/html'
        ctx.body = data
        return;
    }
    if(ctx.url === '/api') {
        await new Promise(resolve => setTimeout(resolve, 3000));
        // ctx.type = 'application/json'
        // ctx.body = JSON.stringify({
        //     "name": "Lebron James",
        //     "age": 39
        // })
        ctx.body = JSON.stringify({
            "name": "Lebron James",
            "age": 38
        })
        return
    }
    ctx.body = 'Hello, world!'
})

app.listen(3000)
