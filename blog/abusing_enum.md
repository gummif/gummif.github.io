[Gudmundur F. Adalsteinsson](https://github.com/gummif) -- 2019-06-06 -- [Blog index](http://gummif.github.io/blog)

# (Ab)using enum class for strong type aliases

https://quuxplusone.github.io/blog/2018/06/12/perennial-impossibilities/
https://en.cppreference.com/w/cpp/language/enum
https://blog.demofox.org/2015/02/05/getting-strongly-typed-typedefs-using-phantom-types/
https://groups.google.com/a/isocpp.org/forum/#!topic/std-proposals/Y-7cdQcNDrk
https://foonathan.net/blog/2016/10/19/strong-typedefs.html
http://nullptr.nl/2018/02/phantom-types/
https://foonathan.net/blog/2016/10/11/type-safe.html#comments
https://github.com/foonathan/type_safe/blob/master/include/type_safe/integer.hpp
https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(j:1,lang:c%2B%2B,source:'++++enum+class+Apple%3B%0A++++enum+class+Orange%3B%0A++++Apple+a%7B3%7D%3B%0A++++Orange+o%7B5%7D%3B%0A++++//o+%3D+a%3B+++//+doesn!'t+compile%0A++++Apple+fruit_salad%7BOrange%7B2%7D%7D%3B+//+compiles+without+warning!!%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:33.333333333333336,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:compiler,i:(compiler:g73,filters:(b:'0',binary:'1',commentOnly:'0',demangle:'0',directives:'0',execute:'1',intel:'0',trim:'0'),lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17',source:1),l:'5',n:'0',o:'x86-64+gcc+7.3+(Editor+%231,+Compiler+%231)+C%2B%2B',t:'0')),k:33.333333333333336,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compiler:1,editor:1),l:'5',n:'0',o:'%231+with+x86-64+gcc+7.3',t:'0')),k:33.33333333333333,l:'4',n:'0',o:'',s:0,t:'0')),l:'2',n:'0',o:'',t:'0')),version:4


arithmetic: affine space/vector space, (torsor)


