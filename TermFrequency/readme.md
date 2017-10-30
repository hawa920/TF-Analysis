## Term Frequency
### 說明
> 給定一文本(article)，以及一詞單(word list)，統計詞單中的字詞在文本中的出現頻率。


### 作法
> 1. 先把詞單中的字詞一一丟進hash function計算出一hash value，並建立一張hash table
![](https://i.imgur.com/pA75iwu.png)
> 2. 定義出這些詞的長度範圍(如2 ～ 7 個中文字)
> 3. 把文本(article)讀進來，從頭 traverse 一遍，實做 Rabin-Karp Rolling hash 可以完成統計長度為 k 的所有詞的頻率。


**Note：**  當m為2的冪次方減1，a%(m+1) = a & m，且遵守分配律。


` cur_hv = (((cur_hv * RK_RHB) + (int) text_ptr[idx_i + key_len]) - (int) text_ptr[idx_i] * base_power) & RK_RHM;`

### 優化
> 1. 用 bit-wise and  取代 module 運算。
> 2. 記憶體允許況下讓 hash table size > 詞的數量 n （減少碰撞率）
> 3. 選用 29、31、33、127  等數字作為 hash base （良好hash分佈）
> 4. 多線程執行，依照詞的長度去做分工，(用 i-th 個 thread 去統計所有長度為 i 的詞頻


**比起切割檔案，切割詞長可以避免race condtion。 目前尚未想到如何處理切割檔案帶來的問題**

### 待解決
> 1. n-gram、長詞優先(longest match, n-gram algorithm)
> 2. 切割檔案多工的問題(race condition problems)
> 3. 當檔案大到無法一次讀進記憶體的問題(File partitioning & external sort)
> 4. TF-IDF 


### 操作說明
> 1. 編譯 multi-thread-by-length-of-word.c 
> 2. 從 argv  給入 key list 檔案路徑以及 article 檔案路徑
> 3. 程式將產出詞頻統計結果


### 實測
> 1. 在600MB的文本當中統計8萬個詞的詞頻(長度2-7個中文字)，單線程約 37 秒
> 2. 在600MB的文本當中統計8萬個詞的詞頻(長度2-7個中文字)，6個線程(長度2～長度7)約19秒


**時間的降低幅度並不理想，還有帶釐清是什麼讓速度降不下來，初步猜測可能跟記憶體頻寬有關**


### Demo
> 從 17 MB的文本中(./tds/mid-td)統計12個詞的詞頻(./tds/keylist)，詞的長度為2-7個中文字。
> Single Thread


`gcc test-rabin-karp-rolling-hash.c`


`time ./a.out ./tds/keylist ./tds/mid-td`

![](https://i.imgur.com/InHR1pq.png)

> 多工(6 threads, 每一個線程負責所有長度為 k 的統計)

`gcc -pthread multi-thread-by-length-of-word.c`


`time ./a.out ./tds/keylist ./tds/mid-td`

![](https://i.imgur.com/OzjXEQx.png)


### 比較
此一作法在詞的量很大的時候，比起 strstr (kmp algorithm)會更有效率，以下為時間複雜度分析，

n：文本(article)長度


m：詞的量(number of keys)


使用 strstr (kmp)的時間複雜度為 O( n * m )

而上面的方法首先將 m 映射到一個較小的集合k，k由 m的長度組成。

舉例而言 "蘋果、小貓、小狗、老鼠、動物、天氣"，總共有6個單字，但是映射到 k之後，就只剩1個元素(length = 2 wchar)

時間複雜度為 O( k * m )


然而當今天只有少數幾個單字要統計詞頻，使用 strstr 的效果自然會比較好。


`gcc strstr-ver.c`


`time ./a.out ./tds/keylist ./tds/mid-td`

![](https://i.imgur.com/zd0r3dj.png)


### 結論
最好根據不同狀況決定要用那一種演算法來實做string matching，各有優缺。
