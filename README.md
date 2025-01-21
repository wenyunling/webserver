- 一个C++11优化过的代码，[原项目](https://github.com/qinguoyi/TinyWebServer)
- 使用方法：
  - 建立数据库
    ```C++
    // 建立yourdb库
    create database yourdb;

    // 创建user表
    USE yourdb;
    CREATE TABLE user(
        username char(50) NULL,
        passwd char(50) NULL
    )ENGINE=InnoDB;

    // 添加数据
    INSERT INTO user(username, passwd) VALUES('name', 'passwd');
    ```
  - 构建代码
    ```sh
    make server
    ./server 9006
    ```


