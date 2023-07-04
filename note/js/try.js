try {
    debugger
    throw new TypeError("in try type error");
} catch (err) {
    console.log(err.name); // "TypeError"
    console.log(err.message); // "oops"
    throw new TypeError("in catch type error")
} finally {
    console.log('finally')
}

/**
 * a: stm_try
 * [a=stm_block, b=ast_identifier, c=stm_block, d=stm_block]
 * a: stm_block -> a: ast_list [a=stm_throw, b=0, c=0, d=0]
 * b: ast_identifier -> "err"
 * c: stm_block -> a: ast_list [a=exp_call, b=ast_list[a=exp_call,bcd=0], c=0, d=0]
 * d: stm_block -> a: ast_list [a=exp_call]
 * 
 * op_try 
 * jump [begin try] (setjmp({buf, pc}) js_trypc保存当前pc到trybuf中, try code中throw使用这个pc填充当前pc，执行下面的catch代码)
 * begin catch
 * catch_code
 * end_catch
 * jump [end try]
 * begin try
 * try code (里面可能有throw->longjmp({trybuf}))
 * end try
 * finally code
 */