/*
 * Copyright (C) 2011-2017 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <libcxx/string>
#include <libcxx/vector>

#include "../Enclave.h"
#include "Enclave_t.h"
#include "sgx_tprotected_fs.h"

struct faqRec
{
    std::string readHead;
    std::string readSeq;
    std::string readQual;
};

static const char b2p[256] = {
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', //0
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N',
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', //1
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N',
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', //2
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N',
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', //3
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N',
    'N', 'T', 'N', 'G', 'N', 'N', 'N', 'C', //4   'A' 'C' 'G'
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N',
    'N', 'N', 'N', 'N', 'A', 'N', 'N', 'N', //5   'T'
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N',
    'N', 'T', 'N', 'G', 'N', 'N', 'N', 'C', //6   'a' 'c' 'g'
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N',
    'N', 'N', 'N', 'N', 'A', 'N', 'N', 'N', //7   't'
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N',
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', //8
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N',
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', //9
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N',
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', //10
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N',
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', //11
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N',
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', //12
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N',
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', //13
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N',
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', //14
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N',
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', //15
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N'};

std::vector<std::vector<bool> *> bloom_filters;

int bmer = 0;
int bmer_step = 0;
int nhash = 0;
int se = 0;
int fq = 0;
int pnum = 0;

// MurmurHash2, 64-bit versions, by Austin Appleby
// https://sites.google.com/site/murmurhash/MurmurHash2_64.cpp?attredirects=0
uint64_t MurmurHash64A(const void *key, int len, unsigned int seed)
{
    const uint64_t m = 0xc6a4a7935bd1e995;
    const int r = 47;

    uint64_t h = seed ^ (len * m);

    const uint64_t *data = (const uint64_t *)key;
    const uint64_t *end = data + (len / 8);

    while (data != end)
    {
        uint64_t k = *data++;

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const unsigned char *data2 = (const unsigned char *)data;

    switch (len & 7)
    {
    case 7:
        h ^= uint64_t(data2[6]) << 48;
    case 6:
        h ^= uint64_t(data2[5]) << 40;
    case 5:
        h ^= uint64_t(data2[4]) << 32;
    case 4:
        h ^= uint64_t(data2[3]) << 24;
    case 3:
        h ^= uint64_t(data2[2]) << 16;
    case 2:
        h ^= uint64_t(data2[1]) << 8;
    case 1:
        h ^= uint64_t(data2[0]);
        h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}

void filInsert(std::vector<std::vector<bool> *> &myFilters, const unsigned pn, const std::string &bMer)
{
    for (int i = 0; i < nhash; ++i)
        (*myFilters[pn])[MurmurHash64A(bMer.c_str(), bmer, i) % myFilters[pn]->size()] = true;
}

bool filContain(const std::vector<std::vector<bool> *> &myFilters, const unsigned pn, const std::string &bMer)
{
    for (int i = 0; i < nhash; ++i)
    {
        if (!myFilters[pn]->at(MurmurHash64A(bMer.c_str(), bmer, i) % myFilters[pn]->size()))
            return false;
    }
    return true;
}

void getCanon(std::string &bMer)
{
    int p = 0, hLen = (bmer - 1) / 2;
    while (bMer[p] == b2p[(unsigned char)bMer[bmer - 1 - p]])
    {
        ++p;
        if (p >= hLen)
            break;
    }
    if (bMer[p] > b2p[(unsigned char)bMer[bmer - 1 - p]])
    {
        for (int lIndex = p, rIndex = bmer - 1 - p; lIndex <= rIndex; ++lIndex, --rIndex)
        {
            char tmp = b2p[(unsigned char)bMer[rIndex]];
            bMer[rIndex] = b2p[(unsigned char)bMer[lIndex]];
            bMer[lIndex] = tmp;
        }
    }
}

void dispatchRead(char *sequence1, int seq1_len, char *sequence2, int seq2_len)
{
    size_t buffSize = 4000000;
    std::vector<std::string> rdFiles(pnum, "");

    std::vector<char *> sequences;
    sequences.push_back(sequence1);

    if (!se)
    {
        sequences.push_back(sequence2);
    }

    std::string msFile;
    std::string imdFile;

    size_t fileNo = 0, readId = 0;
    std::string readHead, readSeq, readDir, readQual, rName;

    char delim[] = "\n";

    bool readValid = true;
    while (readValid)
    {
        readValid = false;
        // set up readBuff
        std::vector<faqRec> readBuffer; // fixed-size to improve performance
        char *line = strtok(sequences[fileNo], delim);
        while (line != nullptr)
        {
            readHead = line;
            line = strtok(NULL, delim);
            if (line != nullptr)
            {
                readSeq = line;
            }
            else
            {
                printf("FATAL : Null sequence");
            }
            std::transform(readSeq.begin(), readSeq.end(), readSeq.begin(), ::toupper);
            line = strtok(NULL, delim);
            if (line != nullptr)
            {
                readDir = line;
            }

            line = strtok(NULL, delim);
            if (line != nullptr)
            {
                readQual = line;
            }

            readHead[0] = ':';
            faqRec rRec;

            std::string hstm;
            if (!fq)
            {
                hstm.append(">").append(std::to_string(readId)).append(readHead);
            }
            else
            {
                hstm.append("@").append(std::to_string(readId)).append(readHead);
            };
            rRec.readHead = hstm;
            rRec.readSeq = readSeq;
            rRec.readQual = readQual;
            readBuffer.push_back(rRec);
            if (!se)
                fileNo = (fileNo + 1) % 2;
            ++readId;
            if (readBuffer.size() == buffSize)
                break;

            //printf("readHead : %s\n", hstm.c_str());
            //printf("readSeq : %s\n", readSeq.c_str());

            line = strtok(NULL, delim);
            if (line == nullptr && !se)
            { // move to next file
                line = strtok(sequences[fileNo], delim);
            }
        }

        printf("Done filling buffers\n");
        if (readBuffer.size() == buffSize)
            readValid = true;

        //dispatch buffer
        int pIndex;
        std::vector<bool> dspRead(buffSize, false);
        for (pIndex = 0; pIndex < pnum; ++pIndex)
        {
            //printf("Processing partition %d having %d buffers\n", pIndex, readBuffer.size());
            for (size_t bIndex = 0; bIndex < readBuffer.size(); ++bIndex)
            {
                faqRec bRead = readBuffer[bIndex];
                size_t readLen = bRead.readSeq.length();
                //size_t j=0;
                for (size_t j = 0; j <= readLen - bmer; j += bmer_step)
                {
                    //printf("Processing bmer %d\n", j);
                    std::string bMer = bRead.readSeq.substr(j, bmer);
                    //printf("Get canon...\n");
                    getCanon(bMer);
                    //printf("Checking bloomfilter v2... %s\n", bMer);
                    // todo optimize here
                    if (filContain(bloom_filters, pIndex, bMer))
                    {
                        //printf("Checked bloomfilter...\n");
                        dspRead[bIndex] = true;
                        if (!fq)
                            rdFiles[pIndex].append(bRead.readHead).append("\n").append(bRead.readSeq).append("\n");
                        else
                            rdFiles[pIndex].append(bRead.readHead).append("\n").append(bRead.readSeq).append("\n+\n").append(bRead.readQual).append("\n");
                        break;
                    }
                }
            }
            printf("End of outer while loop\n");
        } // end dispatch buffer
        for (size_t bIndex = 0; bIndex < readBuffer.size(); ++bIndex)
        {
            if (!dspRead[bIndex])
                msFile.append(readBuffer[bIndex].readHead.substr(1, std::string::npos))
                    .append("\t4\t*\t0\t0\t*\t*\t0\t0\t*\t*\n");
        }
    }

    imdFile.append(std::to_string(readId)).append("\n");

    //printf("\n\n");
    //printf("msFile content : \n\n");
    //printf(msFile.c_str());
    //printf("\n\n");

    //printf("imFile content : \n\n");
    std::string max_inf = "maxinf";
    ocall_print_file(imdFile.c_str(), max_inf.c_str(), 1);
    //printf(imdFile.c_str());
    //printf("\n\n");

    printf("printing %d rdFiles %d...\n", rdFiles.size(), pnum);
    for (int i = 0; i < rdFiles.size(); i++)
    {
        //printf("rdFile %d content : \n\n", i);
        //printf(rdFiles[i].c_str());
        //printf("\n\n");

        std::string file_name = "mread-" + std::to_string(i) + ".fa";

        //sealing
        printf("sealing size %d of rdFiles[%d]...\n", rdFiles[i].size(), i);
        size_t plaintext_len = rdFiles[i].size() + 1;
        uint8_t *plaintext = (uint8_t *)rdFiles[i].c_str();
        size_t sealed_size = sizeof(sgx_sealed_data_t) + plaintext_len;
        uint8_t *sealed_data = (uint8_t *)malloc(sealed_size);
        sgx_status_t status = seal(plaintext, plaintext_len, (sgx_sealed_data_t *)sealed_data, sealed_size);

        ocall_print_file((const char *)sealed_data, file_name.c_str(), 0);
        //ocall_print_file(rdFiles[i].c_str(), file_name.c_str(), 0);
    }
}

void ecall_start_dispatch(int para_bmer,
                          int para_bmer_step,
                          int para_nhash,
                          int para_se,
                          int para_fq,
                          int para_pnum)
{
    bmer = para_bmer;
    bmer_step = para_bmer_step;
    nhash = para_nhash;
    se = para_se;
    fq = para_fq;
    pnum = para_pnum;

    printf("Initialized dispatch with pnum %d\n", pnum);
}

void ecall_finalize_dispatch()
{
    //delete data;
}

void ecall_load_encrypted_data(char *encrypted_data, size_t encrypted_size, int seq_len)
{
    // do decryption
    char *data_seq = (char *)malloc(seq_len + 1);
    char *plaintext = data_seq;
    uint32_t plaintext_len = seq_len + 1;
    decryptMessage(encrypted_data, encrypted_size, plaintext, plaintext_len);
    plaintext[seq_len] = '\0';
    printf("in enclave, plaintext_len: %ld\n", plaintext_len);

    printf("Dispatching read of length : %d, pnum: %d\n", seq_len, pnum);
    //(data_seq, seq_len, nullptr, 0);
    dispatchRead(data_seq, plaintext_len, nullptr, 0);
}

void ecall_load_sealed_data(sgx_sealed_data_t *sealed_data, size_t sealed_size, int seq_len)
{
    // do decryption
    char *data_seq = (char *)malloc(seq_len + 1);
    uint8_t *plaintext = (uint8_t *)data_seq;
    uint32_t plaintext_len = seq_len + 1;
    unseal(sealed_data, sealed_size, plaintext, plaintext_len);
    printf("in enclave, plaintext_len: %ld\n", plaintext_len);

    printf("Dispatching read of length : %d, pnum: %d\n", seq_len, pnum);
    //(data_seq, seq_len, nullptr, 0);
    dispatchRead(data_seq, plaintext_len, nullptr, 0);
}

void ecall_load_data(char *data_seq, int seq_len)
{
    printf("Dispatching read of length : %d, pnum: %d\n", seq_len, pnum);
    dispatchRead(data_seq, seq_len, nullptr, 0);
}

void ecall_load_sealed_data2(sgx_sealed_data_t *sealed_data1, size_t sealed_size1, int len1, sgx_sealed_data_t *sealed_data2, size_t sealed_size2, int len2)
{
    // do decryption of both seq here
    char *data1 = (char *)malloc(len1 + 1);
    uint8_t *plaintext1 = (uint8_t *)data1;
    uint32_t plaintext_len1 = len1 + 1;
    unseal(sealed_data1, sealed_size1, plaintext1, plaintext_len1);

    char *data2 = (char *)malloc(len2 + 1);
    uint8_t *plaintext2 = (uint8_t *)data2;
    uint32_t plaintext_len2 = len2 + 1;
    unseal(sealed_data2, sealed_size2, plaintext2, plaintext_len2);

    printf("Dispatching paired read of length : %d and %d, pnum: %d\n", len1, len2, pnum);
    dispatchRead(data1, len1, data2, len2);
}

void ecall_load_data2(char *data1, int len1, char *data2, int len2)
{
    // do decryption of both seq here
    printf("Dispatching paired read of length : %d and %d, pnum: %d\n", len1, len2, pnum);
    dispatchRead(data1, len1, data2, len2);
}

void ecall_load_bf(unsigned char *data, long len, long bf_len)
{
    printf("adding a bloom filter of size : %d\n", len);
    printf("\n\n Received Chars\n\n");
    for (int i = 0; i < 5; i++)
    {
        printf("%d,", data[i]);
    }
    printf("\n");

    std::vector<bool> *bf = new std::vector<bool>();
    bf->reserve(bf_len);
    for (long i = 0; i < len; i++)
    {
        unsigned char chr = data[i];
        for (int b = 7; b > -1 && bf->size() < bf_len; b--)
        {
            bf->push_back((chr & (0b00000001 << b)) != 0);
        }
    }

    printf("\n\n Received\n\n");
    for (int i = 0; i < 40; i++)
    {
        printf("%s", bf->at(i) ? "1" : "0");
    }
    printf("\nAdding to the bloom filters...\n");
    bloom_filters.push_back(bf);
    printf("\nAdded to the bloom filters...\n");

    printf("Deleting received char\n");
}

void ecall_print_bf_summary()
{
    printf("No of bloom filters : %d\n", bloom_filters.size());
    for (int i = 0; i < bloom_filters.size(); i++)
    {
        printf("\tSize of bf %ld\n", bloom_filters[i]->size());
    }
}