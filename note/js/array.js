var i = 0
i += 1
// var o = { 'name': 'James', 'age': 38 }
/**
 * exp_object [a:ast_list, bcd=0]
 * a:ast_list [a:exp_prop_val, b:ast_list[a:exp_prop_val, bcd=0], cd=0]
 * a:exp_prop_val [a:exp_string, b:exp_string/exp_number]
 */
// var a = [1, , 'hello']
// 第二个参数类型为EXP_ELISION

/**
 * [1, 'hello']
 * exp_array [a:ast_list, bcd=0]
 * a:ast_list[a:exp_number, b:ast_list[exp_string], cd=0]
 */