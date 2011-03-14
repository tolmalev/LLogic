#ifndef FILESTRUCTS_H
#define FILESTRUCTS_H


struct DocumentHeader{
    int doc_type;
    int name_length;
    int elements_count;
};

struct ElementHeader{
    int in_cnt, out_cnt;
    int x, y, width, height;
    int type;
};

#endif // FILESTRUCTS_H
