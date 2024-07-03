.# Disk Hash Table


## 使用方法
先make file

### add [filename]
參數檔案內 Key, Value 加入於 Disk Hash
#### 命令格式
./disk add example.txt

### query [key]
查詢所指定 key 的資料，如果有找到則是顯示其 value，如果找不到則是顯示 “NOT FOUND”。
#### 命令格式
./disk del key

### import[filename]
將所指定檔案名稱的內容加入 Disk Hash，Key 則是檔案名稱 (不含路徑)，成功後請顯示 “OK”、檔案名稱和檔案大小 (bytes)，例如：”OK test.doc 12345”， 錯誤則是顯示 “FAILUER”。
#### 命令格式
./disk import filename

###  export [key] [save filename]
將所指定 key 的資料保存在指定檔案裏面，成功後請顯示
“OK”、檔案名稱和保存檔案大小 (bytes)，例如：”OK test.doc 12345”。
資料找不到則是顯示 “NOT FOUND”，或者其他錯誤則是顯示 “FAILUER”。
#### 命令格式
./disk export key filename
