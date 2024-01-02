## 数据库支持情况
比较而言，postgresql更加容易理解，文档也更加详细

### [postgresql](https://www.postgresql.org/docs/current/collation.html)
```shell
# 显示系统支持和自定义的collation
\dO+ 
select * from pg_collation; #(mysql系列使用show collation;)
```
postgresql系统支持有两种collation，libc和ICU，前者使用数据库编译时操作系统支持的collation，后者是引入第三方库icu，扩展性更高，docker镜像编译时默认带入了icu
glibc默认排序规则
```shell
echo -en "a\nA\n一\n测\n123\n" | LC_COLLATE="C" sort
123
A
a
一
测

echo -en "a\nA\n一\n测\n123\n" | LC_COLLATE="en_US.UTF-8" sort
123
a
A
一
测

echo -en "A\na\n一\n测\n123\n" | LC_COLLATE="zh_CN.UTF-8" sort
123
测
一
a
A
```
```shell
pg_dump -d postgres -t test -h localhost -Upostgres  # 打印出test表的DDL；（mysql系列使用show create table test）
create collation zh_num_upper (provider = icu, locale = 'zh-u-kn-kf-upper-kr-digit-latn'); # (mysql系列不支持create collation)
#自定义一个collation，使用icu提供的'zh-u-kn-kf-upper-kr-digit-latn'规则排序，kf表示大小写字母排序自定义，kr表示reorder各种字符顺序，总体表示使用zh的locale，大写字母在前，整体顺序是数字，拉丁字符，中文拼音
select name from (values ('123'), ('1'),('a'),('A'),('一'),('测')) _(name) order by name COLLATE "zh-u-kn-kf-upper-kr-digit-latn";  # mysql系列不支持values这种方式
```

### [mariadb](https://www.flokoe.de/posts/database-character-sets-and-collations-explained/)
```shell
# 默认支持的collation
show collation;
#默认支持的字符集，包括字符集默认的collation，gbk默认的是gbk_chinese_ci, utf8mb4默认的collation是utf8mb4_general_ci
show character set;

create table test(name character set utf8mb4 collation utf8mb4_unicode_ci not null);
insert into test values ('1'), ('A'), ('a'), ('一'), ('变'), ('导'), ('测'), ('用');
select * from test order by name CONVERT(name using gbk);
```

## 业务代码支持
```java
<dependencies>
    <dependency>
        <groupId>com.ibm.icu</groupId>
        <artifactId>icu4j</artifactId>
        <version>68.1</version>
    </dependency>

    <dependency>
        <groupId>com.ibm.icu</groupId>
        <artifactId>icu4j-charset</artifactId>
        <version>68.1</version>
    </dependency>

    <dependency>
        <groupId>com.ibm.icu</groupId>
        <artifactId>icu4j-localespi</artifactId>
        <version>68.1</version>
    </dependency>
</dependencies>


import com.ibm.icu.text.Collator;
import com.ibm.icu.util.ULocale;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

public class Demo {
    public static void main(String[] args) {
        Collator col = null;
        try {
            col = Collator.getInstance(Locale.CHINA);
        } catch (Exception e) {

        }

        List<String> l = new ArrayList<String>();
        l.add("123");
        l.add("2");
        l.add("A");
        l.add("a");
        l.add("一");
        l.add("测");
        l.sort(col);
        System.out.format("icu中Locale.CHINA结果: %s\n", l);
        java.text.Collator c = java.text.Collator.getInstance(Locale.CHINA);
        l.sort(c);
        System.out.format("java原生Locale.CHINA结果: %s\n", l);
        Collator cus = Collator.getInstance(new ULocale("zh-u-kr-Latn-Hani-digit"));
        l.sort(cus);
        System.out.format("icu中custom locale结果: %s\n", l);

        byte[] b = col.getCollationKey("一").toByteArray();
        for(byte item : b) {
            System.out.format("%x ", item);
        }
    }
}

icu中Locale.CHINA结果: [123, 测, 一, a, A]
java原生Locale.CHINA结果: [123, a, A, 测, 一]
icu中custom locale结果: [123, a, A, 测, 一]
97 39 1 5 1 5 0 
```
```javascript
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/Collator
['一', 123, 2, 'a', 'A', '测','].sort(new Intl.Collator('zh').compare)
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/Collator/Collator
Intl.Collator(locales, options)
// 初始化文档说明很清晰，locales后面后说明，根据BCP 47文档来定义的，可惜现在还不支持reorder(kr)，options可配置项也很丰富
```

## 国际化
综上所描述的各个层面的排序支持和结果情况，结果可以说及其不统一。排序是国际化概念中的一个分支，使用统一的排序算法支持的(https://unicode.org/reports/tr10/)，并且基于此有有一个ICU库实现(https://unicode-org.github.io/icu/userguide/collation/)，为什么推荐使用ICU统一实现各个层的排序呢？

清晰的规则，icu基于标准，规则可解释，也方便定制
一致的规则，比如glibc各个版本的排序规则就不统一，如果postgres使用libc规则，不同的glibc版本可能出现排序不一致的情况
postgres使用icu，业务代码中可以使用icu，保证了各个微服务模块统一的排序，不然各个微服务使用自己理解的排序，导致终端服务排序不一致（mysql目前不支持icu collation）
易于扩展，比如如果按照中文偏旁或者bpmf排序怎么办，而且这些规则都是正常的规则；比如出海（国际化），不仅排序，currency，datetime等规则也有这个需要

## 知识扩展
### Locale
每个linux系统都需要设置Locale，glibc 和应用程序、函数库库使用区域设置显示本地化的文字、货币、时间、日期、特殊字符等包含地域属性的内容。
```shell
man 5 locale
man 7 locale
LC_COLLATE="zh_CN.UTF-8" sort <<< $'a\nA\n一\n测\n123\n2\n'
LC_COLLATE="C" sort <<< $'a\nA\n一\n测\n123\n2\n'
```
```c
#include <stdio.h>
#include <string.h>
#include <locale.h>
int main()
{
    setlocale(LC_COLLATE, "en_US.UTF-8");
    const char* s1 = "A";
    const char* s2 = "a";

    int s1_len = 1+strxfrm(NULL, s1, 0);
    int s2_len = 1+strxfrm(NULL, s2, 0);

    char t1[s1_len];
    char t2[s2_len];
    //compare original string with strcoll
    printf("strcoll: %d\n", strcoll(s1,s2));
    //compare original string
    printf("strcmp: %d\n",strcmp(s1,s2));
    strxfrm(t1,s1,sizeof(t1));
    strxfrm(t2,s2,sizeof(t2));
    //compare transfered string
    printf("strcmp transfered: %d\n", strcmp(t1,t2));

    printf("A: ");
    for(int i=0; i<s1_len; i++)
        printf("%x ", t1[i]);
    printf("\n");

    printf("a: ");
    for(int i=0; i<s2_len; i++)
        printf("%x ", t2[i]);
    printf("\n");

    return 0;
}
```

### Language Tags
icu库里面使用Locale初始化Collator，icu中Locale概念与POSIX定义的Locale是不一样的，POSIX定义的是[language][_TERRITORY][.CODESET][@modifier]，icu中的没有codeset相关的设置。现在icu中使用language tag标识locale，而且她具有可扩展性，包括各种tailoring设置等。在HTML的lang设置或者http中的accept-language等都使用标准的BCP 47定义的language tag。

网上的BCP 47的定义的rfc文档比较细节，建议看其他入门后可以再看，这里主要推荐先看https://www.w3.org/International/articles/language-tags/，主要描述性讲述language tags（结合https://www.iana.org/assignments/language-subtag-registry/language-subtag-registry）。

格式：language[-extlang][-script][-region][-variant][-extension][-privateuse]

1. language，也叫 the primary language subtag，比如en表示English，ISO 639描述了language code，可以在上面的language subtag registry中查看类别为language的item，使用2个或者3个小写字符表示，比如
```
%%
Type: language
Subtag: es
Description: Spanish
Description: Castilian
Added: 2005-10-16
Suppress-Script: Latn
%%
```
2. extlang，也叫 the extended language subtag，比如zh-yue表示粤语，使用3个小写字符表示。一般情况下，如果使用单个language可以表示的尽量不适用，比如yue可以单独表示language，替换zh-yue
```
%%
Type: language
Subtag: yue
Description: Yue Chinese
Description: Cantonese
Added: 2009-07-29
Macrolanguage: zh
%%
```
3. script，也叫the script subtag，比如zh-Hans中的Hans表示简体中文，表示书写方式，后面的region表示表达方式，比如HK等，同样在IANA中也有相关描述。四个字符表示，首字符大写，比如Hans，Hant，Latn
4. region，也叫the region subtag，比如zh-Hans-HK，表示Traditional Chinese as used in Hong Kong。使用2个大写字符或者3位数字表示
5. variant，这个没用过，Variant subtags are values used to indicate dialects or script variations not already covered by combinations of language, script and region subtag
6. extension和privateuse，extension以一个注册的字符开头，比如u表示unicode相关设置，这个在icu中用到很多，比如zh-u-kn-kf-upper-kr-digit-latn，zh是language，u开始时unicode的extension；privateuse是自定义使用，通常以x开始，意义自己解释

### ICU中language tag扩展unicode
很多引用icu的库，比如Chromium就是使用类似的定义，javascript中的Intl.Locale初始化参数就是类似，还有postgresql中定义collation的locale参数也是如此。
扩展注册见https://www.iana.org/assignments/language-tag-extensions-registry/language-tag-extensions-registry，扩展的各个item见https://unicode.org/reports/tr35/tr35-collation.html#Collation_Tailorings
这些扩展都有对应的rule，比如kf用于设置是否大小写顺序，en-u-kf-upper
kf	upper	[caseFirst upper]
上表中的[caseFirst upper]可以写入rule table文件中，这个文件用于判断各个字符的顺序，可以通过python里面的语句获取
```python
zh_collator = icu.Collator.createInstance(icu.Locale('zh-CN'))
zh_collator.getRules()
sorted(['a', 'A'], key = zh_collator.getSortKey)

rules = icu.Collator.createInstance(icu.Locale('zh-CN')).getRules()
rules = '[caseFirst upper]' + rules
zh_rule_coll = icu.RuleBasedCollator(rules)
sorted(['a', 'A'], key = zh_rule_coll.getSortKey)
```

### Unicode Collation Algorithm（UCA）
Unicode提供默认的顺序给CLDR的root collation，用于各种语言，locale或者其他配置tailor，https://unicode.org/reports/tr10/#Order_DUCET
Unicode字符比较受到多个方面的影响，比如语言(同样两个字符，不同国家排序不一样)，使用场景(德国电话本和字典里面的字符顺序可能不一样)，自定义(大小写顺序)等，为了描述这些复杂性，使用多层结构描述，见下表

|Level|	Description	|Examples|
|--|--|--|
|L1|	Base characters	|role < roles < rule
|L2|	Accents	|role < rôle < roles
|L3|	Case/Variants|	role < Role < rôle
|L4|	Punctuation|	role < “role” < Role
|Ln|	Identical|	role < ro□le < “role”

level与weight通用，这些level在配置中使用strength表示，比如setStrength
以下这段解释了多个名词，DUCET，CLDR（Unicode Common Locale Data Repository，UCA的各种扩展和补充），tailor，Collation Element转化为Sort key便于比较

The Unicode Collation Algorithm (UCA) details how to compare two Unicode strings while remaining conformant to the requirements of the Unicode Standard. This standard includes the Default Unicode Collation Element Table (DUCET), which is data specifying the default collation order for all Unicode characters, and the CLDR root collation element table that is based on the DUCET. This table is designed so that it can be tailored to meet the requirements of different languages and customizations.

Briefly stated, the Unicode Collation Algorithm takes an input Unicode string and a Collation Element Table, containing mapping data for characters. It produces a sort key, which is an array of unsigned 16-bit integers. Two or more sort keys so produced can then be binary-compared to give the correct comparison between the strings for which they were generated.

ICU库使用RuleBasedCollator实现Unicode官方的collation order，具体见文档https://github.com/unicode-org/icu/blob/main/docs/userguide/collation/customization/index.md，这个提供很多细粒度的order设置，比如想把a放在b的后面等

Variable collation elements, which typically include punctuation characters and which may or may not include a subset of symbol characters, require special handling in the Unicode Collation Algorithm. 比如空格，-等这些需要设置特别对待，glibc对于这种17，28版本的默认设置有变化（https://postgresql.verite.pro/blog/2018/08/27/glibc-upgrade.html ）

以下是CLDR的root collation的locale默认tailors，对于中文
```
zh/defaultCollation=pinyin
zh/pinyin
zh/stroke
zh-Hant/defaultCollation=stroke
```
```python
zh_pinyin = icu.Collator.createInstance(icu.Locale('zh'))
zh_stroke = icu.Collator.createInstance(icu.Locale('zh-Hant'))
zh_pinyin.compare('一'，'测') # 1
zh_stroke.compare('一'，'测') # -1
```

https://www.unicode.org/reports/tr35/tr35-collation.html#Setting_Options CLDR中也描述了各种unicode的扩展描述，以及对应的rule syntax

## 引用列表
https://symbl.cc/en/unicode/blocks/cjk-unified-ideographs/ unicode table  
https://www.flokoe.de/posts/database-character-sets-and-collations-explained/ mysql中character set和collation解释  
https://gitlab.pyicu.org/main/pyicu python版本icu仓库  
https://github.com/dverite/icu_ext/tree/master postgresql icu扩展  
https://unicode.org/reports/tr10/ unicode官方collation算法文档  
https://www.unicode.org/reports/tr35/tr35-collation.html CLDR collation algorithm，collation算法的补充文档  
https://peter.eisentraut.org/blog/2023/03/14/how-collation-works collation工作机制描述  
https://peter.eisentraut.org/blog/2023/04/12/how-collation-of-punctuation-and-whitespace-works 同上  
https://peter.eisentraut.org/blog/2023/05/16/overview-of-icu-collation-settings icu中collation设置参数描述  
https://peter.eisentraut.org/blog/2023/06/13/overview-of-icu-collation-settings-part-2 同上  
https://www.postgresql.org/docs/current/collation.html pg中collation官方文档  
https://unicode-org.github.io/icu/userguide/collation/ icu中collation文档  
https://gist.github.com/dpk/8325992 pyicu cheatsheet  
https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/Collator 前端国际化对象Intl的collator支持文档  
https://github.com/unicode-org/icu/blob/main/docs/userguide/collation/customization/index.md icu collation自定义文档  
https://www.rfc-editor.org/rfc/rfc4647.html Unicode中collation的locale标准，html中lang也是使用这个  
https://aticleworld.com/strxfrm-in-c/ glibc strxfrm示例  
https://postgresql.verite.pro/blog/2018/08/27/glibc-upgrade.html glibc升级对pg带来的影响   
https://github.com/awslabs/compat-collation-for-glibc/tree/2.17-326.el7 aws应对glibc不同版本影响collate新建的库  
https://techcommunity.microsoft.com/t5/azure-database-for-postgresql/don-t-let-collation-versions-corrupt-your-postgresql-indexes/ba-p/1978394 collation版本对pg索引的影响  
https://xobo.org/unicode-normalization-nfd-nfc-nfkd-nfkc/ Unicode normalization中文示例  
https://www.w3.org/International/articles/language-tags/ Language tags描述  
https://www.iana.org/assignments/language-subtag-registry/language-subtag-registry Language subtag注册表  
https://www.w3.org/International/questions/qa-choosing-language-tags 如何选择language tags  