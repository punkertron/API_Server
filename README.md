# File Storage service

---
## Table of Contents

- [Features](#features)
- [Subject](#subject)
- [Getting Started](#getting-started)
- [Usage](#usage)
  - [Sign-up](#sign-up)
  - [Authentication](#authentication)
  - [Uploading files](#uploading-files)
  - [Get List of Files](#get-list-of-files)
  - [Delete file](#delete-file)
  - [Download file](#download-file)

---
## Features

- Store and manage files with ease.
- RESTful API for file operations.
- Utilizes C++, Docker, PostgreSQL, pqxx, and Crow.

---
## Getting Started
To start service, simply use the `make` command in your terminal (you may have to install docker and docker-compose).

---
## Subject
The [task](SUBJECT.md) was created by ChatGPT.

---
## Usage

### Sign-up
```curl
curl --request POST 'http://localhost:18081/auth/sign-up' \
--header 'Content-Type: application/json' \
--data-raw '{
    "user":"FooBar",
    "password":"Qwerty123!"
}'
```

Example response:
```json
{
  "id": 1
}
```

### Authentication
```curl
curl --request POST 'http://localhost:18081/auth/sign-in' \
--header 'Content-Type: application/json' \
--data-raw '{
    "user":"FooBar",
    "password":"Qwerty123!"
}'
```


Example response:
```json
{
  "token": "medb9Cfhx.UIBEqaXh7ji2AD0adE9dLjhOkeeqmBp69QyoQjvLsDBS6pT9TQasZi"
}
```

### Uploading files
First, let's create a file:
```bash
echo "Test text" > test.txt
```

Then, let's send the file to the server
```curl
curl --request POST 'http://localhost:18081/api/v1/files' \
--header 'Authorization: Bearer medb9Cfhx.UIBEqaXh7ji2AD0adE9dLjhOkeeqmBp69QyoQjvLsDBS6pT9TQasZi' \
-F "InputFile=@test.txt"
```

Example response:
```json
{
  "file_id": 1
}
```

### Get List of Files
```curl
curl --request GET 'http://localhost:18081/api/v1/files/list' \
--header 'Authorization: Bearer medb9Cfhx.UIBEqaXh7ji2AD0adE9dLjhOkeeqmBp69QyoQjvLsDBS6pT9TQasZi'
```
Example response:
```json
[
  {
    "name": "test.txt",
    "file_id": 1
  }
]
```

### Download file
```curl
curl --request GET 'http://localhost:18081/api/v1/files' \
--header 'Authorization: Bearer medb9Cfhx.UIBEqaXh7ji2AD0adE9dLjhOkeeqmBp69QyoQjvLsDBS6pT9TQasZi' \
--header 'Content-Type: application/json' \
--data-raw '{
    "file_id": 6
}'
```
Example response:
```
Test text
```

### Delete file
```curl
curl --request DELETE 'http://localhost:18081/api/v1/files' \
--header 'Authorization: Bearer medb9Cfhx.UIBEqaXh7ji2AD0adE9dLjhOkeeqmBp69QyoQjvLsDBS6pT9TQasZi' \
--header 'Content-Type: application/json' \
--data-raw '{
    "file_id": 1
}'
```

Example response:

By default, curl does not display the response body when a request is successful (i.e., when it receives a 200 OK response).