//message format for LikeServer sending to PrimaryLikesServer
struct msg {
    uint32_t child_id;
    uint32_t num_likes;
} __attribute((packed));

//message format for the response of the PrimaryLikesServer to a LikesServer
struct return_msg {
    uint32_t return_code;
} __attribute((packed));
