# /dev/twitter
(Format:UTF-8)  
A module to make the /dev/twitter

## 概要
**これはジョークソフトです。常用や他人と共有しているPCでは実行しないでください。**  
/dev/twitterを生やすための怪しいソフトです。  

## ビルド
### 依存関係

* build-essential
* kernel-header
* nkf(Network Kanji Filter)( https://ja.osdn.net/projects/nkf/ )
* curl
* jq
* openssl

dev_twitter.cと同じディレクトリに"key"を作成します。  
https://developer.twitter.com からアプリを作成するか、既存のアプリのトークンを持ってきて以下のように記述します。

```
CONSUMER_KEY=xxxxxxxxxxxxxxxxxxxxxxxx
CONSUMER_SECRET=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
ACCESS_TOKEN=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
ACCESS_TOKEN_SECRET=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
```

### ビルド

```
$ make
$ sudo make load
$ echo "Test" > /dev/twitter
$ sudo make unload # 電源を切る前に行ってください
```

## 仕組み
書かれた内容をハンドラをクローズする際にustomo-tweetをcall_usermodehelperで叩いてるだけです。
かなり危ない作りなので使用は自己責任でお願いします...

## usptomo-tweet

https://github.com/ryuichiueda/TomoTool

```
The MIT License

Copyright (C) 2013-2015 Ryuichi Ueda

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
```
