#ifndef UTIL_H
#define UTIL_H

#include "Config.h"
#include "Store.h"

#include <stdlib.h>

#define NOT_FOUND -1
#define NOT_EQUAL 0
#define EQUAL 1
#define FIRST_TID 0
#define FIRST_DIM 0
#define NO_OVERLAPP 1
#define PARTIAL_OVERLAP 2
#define FULL_OVERLAP 3
//#define NULL (void*) 0

// import all supported SIMD instructions
#include <x86intrin.h>

static const uint32_t BITMASK[16] = {0,     65534, 65532, 65528, 65520, 65504, 65472, 65408,
                                     65280, 65024, 64512, 63488, 61440, 57344, 49152, 32768};
static const unsigned int INT_AND_BITMASK[4] = {15, 1, 3, 7};
static const unsigned int AND_BITMASK[16] = {65535, 1,   3,    7,    15,   31,   63,    127,
                                             255,   511, 1023, 2047, 4095, 8191, 16383, 32767};
static const uint32_t INT_BITMASK[4] = {0, 14, 12, 8};

const static uint32_t LAST_ENTRY_MASK = 2147483648;            // 0b10000000000000000000000000000000
const static uint32_t RECOVER_MASK = 2147483647;               // 0b01111111111111111111111111111111
const static uint64_t LAST_ENTRY_MASK64 = 4611686018427387904; // 0b10000000000000000000000000000000*
const static uint64_t RECOVER_MASK64 = 4611686018427387903;    // 0b01111111111111111111111111111111*

class RangeQueries
{
  public:
    uint32_t **lowerBound;
    uint32_t **upperBound;
    uint32_t NUM_QUERIES, NUM_DIM;

    RangeQueries(uint32_t numQueries, Store *s) : NUM_QUERIES(numQueries), NUM_DIM(s->NUM_DIM)
    {
        lowerBound = new uint32_t *[NUM_QUERIES];
        upperBound = new uint32_t *[NUM_QUERIES];
        uint32_t *randPoint1;
        uint32_t *randPoint2;

        uint32_t seed = 123456;
        srand(seed);
        uint32_t size = s->NUM_POINTS;

        for (uint32_t query = 0; query < NUM_QUERIES; query++)
        {
            randPoint1 = s->getPoint(rand() % size);
            randPoint2 = s->getPoint(rand() % size);
            lowerBound[query] = new uint32_t[NUM_DIM];
            upperBound[query] = new uint32_t[NUM_DIM];

            for (uint32_t dim = 0; dim < NUM_DIM; dim++)
            {
                if (randPoint1[dim] < randPoint2[dim])
                {
                    lowerBound[query][dim] = randPoint1[dim];
                    upperBound[query][dim] = randPoint2[dim];
                }
                else
                {
                    lowerBound[query][dim] = randPoint2[dim];
                    upperBound[query][dim] = randPoint1[dim];
                }
            }
        }
    }

    RangeQueries(uint32_t numQueries, uint32_t begin, uint32_t end, Store *s) :
        NUM_QUERIES(numQueries),
        NUM_DIM(s->NUM_DIM)
    {
        lowerBound = new uint32_t *[NUM_QUERIES];
        upperBound = new uint32_t *[NUM_QUERIES];
        uint32_t *randPoint1;
        uint32_t *randPoint2;

        for (uint32_t query = 0; query < NUM_QUERIES; query++)
        {
            randPoint1 = s->getPoint(begin);
            randPoint2 = s->getPoint(end);
            lowerBound[query] = new uint32_t[NUM_DIM];
            upperBound[query] = new uint32_t[NUM_DIM];

            for (uint32_t dim = 0; dim < NUM_DIM; dim++)
            {
                if (randPoint1[dim] < randPoint2[dim])
                {
                    lowerBound[query][dim] = randPoint1[dim];
                    upperBound[query][dim] = randPoint2[dim];
                }
                else
                {
                    lowerBound[query][dim] = randPoint2[dim];
                    upperBound[query][dim] = randPoint1[dim];
                }
            }
        }
    }
};

class PartialMatchQueries : public RangeQueries
{
  public:
    bool *columnsForSelect;

    PartialMatchQueries(uint32_t numQueries, Store *s, bool *_columnsForSelect) :
        RangeQueries(numQueries, s),
        columnsForSelect(_columnsForSelect)
    {
    }

    void print(uint32_t numQ)
    {
        for (unsigned int numDim = 0; numDim < NUM_DIM; numDim++)
        {
            if (!columnsForSelect[numDim])
                cout << "[ALL];";
            else
                cout << "[" << lowerBound[numQ][numDim] << "-" << upperBound[numQ][numDim] << "];";
        }
    }
};

class EMQueries
{
  public:
    uint32_t **pointsQueries;
    uint32_t NUM_QUERIES, NUM_DIM;

    EMQueries(uint32_t numQueries, Store *s) : NUM_QUERIES(numQueries), NUM_DIM(s->NUM_DIM)
    {
        pointsQueries = new uint32_t *[NUM_QUERIES];
        uint32_t *randPoint1;

        uint32_t seed = 123456;
        srand(seed);
        uint32_t size = s->NUM_POINTS;

        for (uint32_t query = 0; query < NUM_QUERIES; query++)
        {
            int pointNum = rand() % size;
            randPoint1 = s->getPoint(pointNum);
            pointsQueries[query] = new uint32_t[NUM_DIM + 1];

            for (uint32_t dim = 0; dim < NUM_DIM; dim++)
            {
                pointsQueries[query][dim] = randPoint1[dim];
            }
            pointsQueries[query][NUM_DIM] = pointNum;
        }
    }
};

inline uint32_t isEqual(uint32_t *p1, uint32_t *p2, uint32_t length)
{
    for (uint32_t dim = 0; dim < length; dim++)
    {
        if (p1[dim] != p2[dim])
        {
            return NOT_EQUAL;
        }
    }
    return EQUAL;
}

inline uint32_t isEqual(const uint32_t *p1, const uint32_t *p2, const uint32_t length)
{
    for (uint32_t dim = 0; dim < length; dim++)
    {
        if (p1[dim] != p2[dim])
        {
            return NOT_EQUAL;
        }
    }
    return EQUAL;
}

inline bool
isIn(const uint32_t *point, const uint32_t *lowerBoundQuery, const uint32_t *upperBoundQuery, const uint32_t length)
{
    for (uint32_t dim = 0; dim < length; dim++)
    {
        if (lowerBoundQuery[dim] > point[dim] || point[dim] > upperBoundQuery[dim])
            return false;
    }
    return true;
}

inline bool isPartiallyIn(const uint32_t *point,
                          const uint32_t *lowerBoundQuery,
                          const uint32_t *upperBoundQuery,
                          const bool *columnsForSelect,
                          const uint32_t length)
{
    for (uint32_t dim = 0; dim < length; dim++)
    {
        if (columnsForSelect[dim])
        { // only consider this column if true at this dim
            if (lowerBoundQuery[dim] > point[dim] || point[dim] > upperBoundQuery[dim])
            {
                return false;
            }
        }
    }
    return true;
}

inline bool isIn(uint32_t min, uint32_t max, uint32_t value)
{
    return ((value >= min) && (value <= max)) ? true : false;
}

inline unsigned char intToChar(uint32_t b)
{
    return (unsigned char) b;
}

inline bool isIn(uint32_t min, uint32_t max, unsigned char value)
{
    return ((value >= intToChar(min)) && (value <= intToChar(max))) ? true : false;
}

inline void myCopy(uint32_t *from, uint32_t *to, uint32_t length)
{ // TODO memcpy
    for (uint32_t entry = 0; entry < length; entry++)
    {
        to[entry] = from[entry];
    }
}

inline bool isEqualCStore(const int TID, uint32_t *query, const uint32_t LENGTH, vector<uint32_t *> *cols)
{
    uint32_t value;
    uint32_t *column;

    for (uint32_t dim = FIRST_DIM; dim < LENGTH; dim++)
    {
        column = (*cols)[dim];
        value = column[TID];
        if (value != query[dim])
        {
            return false;
        }
    }
    return true;
}

/* binary search */
#define check(Z, X, W)                                                                                                 \
    if ((X) >= bins[W] && (X) < bins[W + 1])                                                                           \
        (Z) = W;
#define left(Z, X, W) if ((X) < bins[W + 1])
#define right(Z, X, W) if ((X) >= bins[W])

inline uint32_t GETBIN64(uint32_t X, uint32_t *bins)
{
    uint32_t Z = 0;
    right(Z, X, 32)
    {
        right(Z, X, 48)
        {
            right(Z, X, 56)
            {
                right(Z, X, 60)
                {
                    right(Z, X, 62)
                    {
                        Z = 62;
                        right(Z, X, 63)
                        {
                            Z = 63;
                        }
                    }
                    check(Z, X, 61) left(Z, X, 60)
                    {
                        Z = 60;
                    }
                }
                check(Z, X, 59) left(Z, X, 58)
                {
                    right(Z, X, 58)
                    {
                        Z = 58;
                    }
                    check(Z, X, 57) left(Z, X, 56)
                    {
                        Z = 56;
                    }
                }
            }
            check(Z, X, 55) left(Z, X, 54)
            {
                right(Z, X, 52)
                {
                    right(Z, X, 54)
                    {
                        Z = 54;
                    }
                    check(Z, X, 53) left(Z, X, 52)
                    {
                        Z = 52;
                    }
                }
                check(Z, X, 51) left(Z, X, 50)
                {
                    right(Z, X, 50)
                    {
                        Z = 50;
                    }
                    check(Z, X, 49) left(Z, X, 48)
                    {
                        Z = 48;
                    }
                }
            }
        }
        check(Z, X, 47) left(Z, X, 46)
        {
            right(Z, X, 40)
            {
                right(Z, X, 44)
                {
                    right(Z, X, 46)
                    {
                        Z = 46;
                    }
                    check(Z, X, 45) left(Z, X, 44)
                    {
                        Z = 44;
                    }
                }
                check(Z, X, 43) left(Z, X, 42)
                {
                    right(Z, X, 42)
                    {
                        Z = 42;
                    }
                    check(Z, X, 41) left(Z, X, 40)
                    {
                        Z = 40;
                    }
                }
            }
            check(Z, X, 39) left(Z, X, 38)
            {
                right(Z, X, 36)
                {
                    right(Z, X, 38)
                    {
                        Z = 38;
                    }
                    check(Z, X, 37) left(Z, X, 36)
                    {
                        Z = 36;
                    }
                }
                check(Z, X, 35) left(Z, X, 34)
                {
                    right(Z, X, 34)
                    {
                        Z = 34;
                    }
                    check(Z, X, 33) left(Z, X, 32)
                    {
                        Z = 32;
                    }
                }
            }
        }
    }
    check(Z, X, 31) left(Z, X, 30)
    {
        right(Z, X, 16)
        {
            right(Z, X, 24)
            {
                right(Z, X, 28)
                {
                    right(Z, X, 30)
                    {
                        Z = 30;
                    }
                    check(Z, X, 29) left(Z, X, 28)
                    {
                        Z = 28;
                    }
                }
                check(Z, X, 27) left(Z, X, 26)
                {
                    right(Z, X, 26)
                    {
                        Z = 26;
                    }
                    check(Z, X, 25) left(Z, X, 24)
                    {
                        Z = 24;
                    }
                }
            }
            check(Z, X, 23) left(Z, X, 22)
            {
                right(Z, X, 20)
                {
                    right(Z, X, 22)
                    {
                        Z = 22;
                    }
                    check(Z, X, 21) left(Z, X, 20)
                    {
                        Z = 20;
                    }
                }
                check(Z, X, 19) left(Z, X, 18)
                {
                    right(Z, X, 18)
                    {
                        Z = 18;
                    }
                    check(Z, X, 17) left(Z, X, 16)
                    {
                        Z = 16;
                    }
                }
            }
        }
        check(Z, X, 15) left(Z, X, 14)
        {
            right(Z, X, 8)
            {
                right(Z, X, 12)
                {
                    right(Z, X, 14)
                    {
                        Z = 14;
                    }
                    check(Z, X, 13) left(Z, X, 12)
                    {
                        Z = 12;
                    }
                }
                check(Z, X, 11) left(Z, X, 10)
                {
                    right(Z, X, 10)
                    {
                        Z = 10;
                    }
                    check(Z, X, 9) left(Z, X, 8)
                    {
                        Z = 8;
                    }
                }
            }
            check(Z, X, 7) left(Z, X, 6)
            {
                right(Z, X, 4)
                {
                    right(Z, X, 6)
                    {
                        Z = 6;
                    }
                    check(Z, X, 5) left(Z, X, 4)
                    {
                        Z = 4;
                    }
                }
                check(Z, X, 3) left(Z, X, 2)
                {
                    right(Z, X, 2)
                    {
                        Z = 2;
                    }
                    check(Z, X, 1) left(Z, X, 0)
                    {
                        Z = 0;
                    }
                }
            }
        }
    }

    return Z;
}

#ifndef NO_SIMD_OPERATIONS
inline void andBitvectors(uint8_t *res1, uint8_t *res2, int numberOfBits)
{
    uint32_t size = (numberOfBits + 7) / 8;
    uint32_t pos;
    __m128i *res1Ptr = reinterpret_cast<__m128i *>(res1);
    __m128i *res2Ptr = reinterpret_cast<__m128i *>(res2);
    for (pos = 0; pos < size / sizeof(__m128i); pos++)
    {
        _mm_store_si128(&res1Ptr[pos], _mm_and_si128(_mm_load_si128(&res1Ptr[pos]), _mm_load_si128(&res2Ptr[pos])));
    }
    for (pos *= sizeof(__m128i); pos < size; pos++)
    {
        res1[pos] &= res2[pos];
    }
}

#else
inline void andBitvectors(uint8_t *res1, uint8_t *res2, uint32_t numberOfBits)
{
    uint32_t size = (numberOfBits + 7) / 8;
    for (uint32_t pos = 0; pos < size; pos++)
    { // 8 bits per uint8_t and
        // cout << (int)res1[pos] << "&" << (int)res2[pos];
        res1[pos] &= res2[pos];
        // cout << "=" << (int)res1[pos] << endl;
    }
}
#endif

/**
 * maxSize = STORE->NUM_POINTS
 */
inline void returnTids(vector<uint32_t> *result, uint8_t *res1, uint32_t maxSize)
{
    result->resize(maxSize);
    uint32_t *result_tids = &((*result)[0]);
    int pos = 0;
    uint32_t i = 0;
    const int TIDS_PER_BYTE = 8;

    for (; i < maxSize / TIDS_PER_BYTE; ++i)
    {
        if (res1[i])
        {
            for (int j = 0; j < TIDS_PER_BYTE; ++j)
            {
                result_tids[pos] = i * TIDS_PER_BYTE + j;
                int bitmask = 1 << j;
                pos += (bitmask & res1[i]) >> j;
            }
        }
    }

    // process remaining byte
    for (; i < (maxSize + 7) / TIDS_PER_BYTE; ++i)
    {
        for (uint32_t j = 0; j < maxSize % TIDS_PER_BYTE; ++j)
        {
            result_tids[pos] = i * TIDS_PER_BYTE + j;
            int bitmask = 1 << j;
            pos += (bitmask & res1[i]) >> j;
        }
    }
    result->resize(pos);
}

inline void addTidsToResult(uint32_t begin, uint8_t *res)
{
    uint32_t offset = begin / 8; // 8 tids per chR
    uint8_t addALL = 255;
    res[offset] = addALL;
    res[offset + 1] = addALL;
}

inline void checkTids_OR(uint32_t *col, uint32_t begin, uint8_t *res, uint32_t lower, uint32_t upper)
{
    uint32_t offset = begin / 8; // 8 tids per chR
    uint8_t toadd = 0;

    if (isIn(lower, upper, col[begin]))
    {
        toadd |= 0b00000001;
    }
    if (isIn(lower, upper, col[begin + 1]))
    {
        toadd |= 0b00000010;
    }
    if (isIn(lower, upper, col[begin + 2]))
    {
        toadd |= 0b00000100;
    }
    if (isIn(lower, upper, col[begin + 3]))
    {
        toadd |= 0b00001000;
    }
    if (isIn(lower, upper, col[begin + 4]))
    {
        toadd |= 0b00010000;
    }
    if (isIn(lower, upper, col[begin + 5]))
    {
        toadd |= 0b00100000;
    }
    if (isIn(lower, upper, col[begin + 6]))
    {
        toadd |= 0b01000000;
    }
    if (isIn(lower, upper, col[begin + 7]))
    {
        toadd |= 0b10000000;
    }
    res[offset] = toadd;

    // zweites byte
    begin += 8;
    toadd = 0;
    if (isIn(lower, upper, col[begin]))
    {
        toadd |= 0b00000001;
    }
    if (isIn(lower, upper, col[begin + 1]))
    {
        toadd |= 0b00000010;
    }
    if (isIn(lower, upper, col[begin + 2]))
    {
        toadd |= 0b00000100;
    }
    if (isIn(lower, upper, col[begin + 3]))
    {
        toadd |= 0b00001000;
    }
    if (isIn(lower, upper, col[begin + 4]))
    {
        toadd |= 0b00010000;
    }
    if (isIn(lower, upper, col[begin + 5]))
    {
        toadd |= 0b00100000;
    }
    if (isIn(lower, upper, col[begin + 6]))
    {
        toadd |= 0b01000000;
    }
    if (isIn(lower, upper, col[begin + 7]))
    {
        toadd |= 0b10000000;
    }
    res[offset + 1] = toadd;
}

inline void checkTids_AND(uint32_t *col, uint32_t begin, uint8_t *res, uint32_t lower, uint32_t upper)
{
    uint32_t offset = begin / 8; // 8 tids per chR
    uint8_t toadd = 0;

    if (isIn(lower, upper, col[begin]))
    {
        toadd |= 0b00000001;
    }
    if (isIn(lower, upper, col[begin + 1]))
    {
        toadd |= 0b00000010;
    }
    if (isIn(lower, upper, col[begin + 2]))
    {
        toadd |= 0b00000100;
    }
    if (isIn(lower, upper, col[begin + 3]))
    {
        toadd |= 0b00001000;
    }
    if (isIn(lower, upper, col[begin + 4]))
    {
        toadd |= 0b00010000;
    }
    if (isIn(lower, upper, col[begin + 5]))
    {
        toadd |= 0b00100000;
    }
    if (isIn(lower, upper, col[begin + 6]))
    {
        toadd |= 0b01000000;
    }
    if (isIn(lower, upper, col[begin + 7]))
    {
        toadd |= 0b10000000;
    }
    res[offset] &= toadd; // XXX macht der das was ich will?

    // zweites byte
    begin += 8;
    toadd = 0;
    if (isIn(lower, upper, col[begin]))
    {
        toadd |= 0b00000001;
    }
    if (isIn(lower, upper, col[begin + 1]))
    {
        toadd |= 0b00000010;
    }
    if (isIn(lower, upper, col[begin + 2]))
    {
        toadd |= 0b00000100;
    }
    if (isIn(lower, upper, col[begin + 3]))
    {
        toadd |= 0b00001000;
    }
    if (isIn(lower, upper, col[begin + 4]))
    {
        toadd |= 0b00010000;
    }
    if (isIn(lower, upper, col[begin + 5]))
    {
        toadd |= 0b00100000;
    }
    if (isIn(lower, upper, col[begin + 6]))
    {
        toadd |= 0b01000000;
    }
    if (isIn(lower, upper, col[begin + 7]))
    {
        toadd |= 0b10000000;
    }
    res[offset + 1] &= toadd; // XXX macht der das was ich will?
}

#ifndef NO_SIMD_OPERATIONS
inline void
SIMDcheck32Tids_AND(__m128i *simd_array, uint32_t beginTID, uint8_t *res, __m128i lowerSIMD, __m128i upperSIMD)
{
    uint32_t offset = beginTID / 32; // 8 tids per chR
    uint32_t toAdd = 0;
    __m128i toComp;

    toComp = _mm_load_si128(&simd_array[0]);
    toAdd |=
        _mm_movemask_ps((__m128) _mm_and_si128(_mm_cmpgt_epi32(toComp, lowerSIMD), _mm_cmplt_epi32(toComp, upperSIMD)));

    toComp = _mm_load_si128(&simd_array[1]);
    toAdd |=
        _mm_movemask_ps((__m128) _mm_and_si128(_mm_cmpgt_epi32(toComp, lowerSIMD), _mm_cmplt_epi32(toComp, upperSIMD)))
        << 4;

    toComp = _mm_load_si128(&simd_array[2]);
    toAdd |=
        _mm_movemask_ps((__m128) _mm_and_si128(_mm_cmpgt_epi32(toComp, lowerSIMD), _mm_cmplt_epi32(toComp, upperSIMD)))
        << 8;

    toComp = _mm_load_si128(&simd_array[3]);
    toAdd |=
        _mm_movemask_ps((__m128) _mm_and_si128(_mm_cmpgt_epi32(toComp, lowerSIMD), _mm_cmplt_epi32(toComp, upperSIMD)))
        << 12;

    toComp = _mm_load_si128(&simd_array[4]);
    toAdd |=
        _mm_movemask_ps((__m128) _mm_and_si128(_mm_cmpgt_epi32(toComp, lowerSIMD), _mm_cmplt_epi32(toComp, upperSIMD)))
        << 16;

    toComp = _mm_load_si128(&simd_array[5]);
    toAdd |=
        _mm_movemask_ps((__m128) _mm_and_si128(_mm_cmpgt_epi32(toComp, lowerSIMD), _mm_cmplt_epi32(toComp, upperSIMD)))
        << 20;

    toComp = _mm_load_si128(&simd_array[6]);
    toAdd |=
        _mm_movemask_ps((__m128) _mm_and_si128(_mm_cmpgt_epi32(toComp, lowerSIMD), _mm_cmplt_epi32(toComp, upperSIMD)))
        << 24;

    toComp = _mm_load_si128(&simd_array[7]);
    ((uint32_t *) res)[offset] &=
        toAdd |
        (_mm_movemask_ps((__m128) _mm_and_si128(_mm_cmpgt_epi32(toComp, lowerSIMD), _mm_cmplt_epi32(toComp, upperSIMD)))
         << 28);
}

inline void SIMDcheckTids_AND(__m128i *simd_array, uint32_t begin, uint8_t *res, __m128i lowerSIMD, __m128i upperSIMD)
{
    uint32_t offset = begin / 8; // 8 tids per chR
    uint8_t toAdd = 0;
    __m128i toComp;

    toComp = _mm_load_si128(&simd_array[0]);
    toAdd |=
        _mm_movemask_ps((__m128) _mm_and_si128(_mm_cmpgt_epi32(toComp, lowerSIMD), _mm_cmplt_epi32(toComp, upperSIMD)));

    toComp = _mm_load_si128(&simd_array[1]);
    res[offset] &=
        toAdd |
        (_mm_movemask_ps((__m128) _mm_and_si128(_mm_cmpgt_epi32(toComp, lowerSIMD), _mm_cmplt_epi32(toComp, upperSIMD)))
         << 4);

    toAdd = 0;

    toComp = _mm_load_si128(&simd_array[2]);
    toAdd |=
        _mm_movemask_ps((__m128) _mm_and_si128(_mm_cmpgt_epi32(toComp, lowerSIMD), _mm_cmplt_epi32(toComp, upperSIMD)));

    toComp = _mm_load_si128(&simd_array[3]);
    res[offset + 1] &=
        toAdd |
        (_mm_movemask_ps((__m128) _mm_and_si128(_mm_cmpgt_epi32(toComp, lowerSIMD), _mm_cmplt_epi32(toComp, upperSIMD)))
         << 4);
}

#endif

class Util
{
  public:
    // static const uint32_t NOT_FOUND = -1;
};
#endif // UTIL_H
