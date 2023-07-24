CREATE SCHEMA server AUTHORIZATION server_account;

CREATE TABLE server.users (
    user_id SERIAL PRIMARY KEY,
    username VARCHAR(100),
    hash_pass VARCHAR(64) NOT NULL,
    hash_token VARCHAR(64)
);

CREATE TABLE server.files_info (
    file_id SERIAL PRIMARY KEY,
    name TEXT NOT NULL,
    is_deleted BOOLEAN DEFAULT false,
    user_id INT,
    CONSTRAINT fk_user_id
        FOREIGN KEY(user_id)
            REFERENCES server.users(user_id)
);
