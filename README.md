## V8 实例

通过这个demo，我们希望掌握V8的实际应用能力，以及相关的C++编程基础


## 怎么运行？

使用CLion或者任何支持Cmake的编译器，直接编译即可。

我这边使用的是Clion，编译后的结果在cmake-build-debug。

## CMakeLists.txt

该文件指定了v8的库文件和静态库，这里需要改成你自己的本地目录：


```
include_directories(/Users/linxiaowu/Github/v8/include)
include_directories(/Users/linxiaowu/Github/v8)

link_directories(
        /Users/linxiaowu/Github/v8/out.gn/x64.release.sample/obj
)
```

另外编译v8是直接引用了v8最后生成的结果，所以编译v8的时候你需要这么使用命令：

`ninja -C out.gn/x64.release.sample v8_monolith`

才可以在`CMakeLists.txt`里这么使用：

```
link_libraries(
        v8_monolith
)
```

更多细节请参考我的博客： [如何正确地使用v8嵌入到我们的C++应用中](https://blog.5udou.cn/blog/Ru-He-Zheng-Que-Di-Shi-Yong-v8Qian-Ru-Dao-Wo-Men-De-CYing-Yong-Zhong-19)