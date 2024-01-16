#pragma once

#include <windows.h>

template <typename T>
struct TSecData
{
    T data;
    unsigned char bKey;
    unsigned char FakePtr1;
    unsigned char FakePtr2;
    unsigned short wChecksum;
};

template <typename T>
class TSecType
{
public:
    TSecType()
    {
        /* this->m_secdata = ZAllocEx<ZAllocAnonSelector>(0x017E3E1C)->Alloc(12); */
        this->m_secdata = reinterpret_cast<TSecData<T>*>(malloc(12));

        this->FakePtr1 = static_cast<unsigned int>(reinterpret_cast<unsigned int>(&this[-0x00003FF8]) + rand());
        this->FakePtr2 = static_cast<unsigned int>(reinterpret_cast<unsigned int>(&this[-0x00003FF8]) + rand());

        this->m_secdata->FakePtr1 = static_cast<unsigned char>(LOBYTE(this->FakePtr1));
        this->m_secdata->FakePtr2 = static_cast<unsigned char>(LOBYTE(this->FakePtr2));

        this->SetData(0);
    }

    TSecType(T data)
    {
        /* this->m_secdata = ZAllocEx<ZAllocAnonSelector>(0x017E3E1C)->Alloc(12); */
        this->m_secdata = reinterpret_cast<TSecData<T>*>(malloc(12));

        this->FakePtr1 = static_cast<unsigned int>(reinterpret_cast<unsigned int>(&this[-0x00003FF8]) + rand());
        this->FakePtr2 = static_cast<unsigned int>(reinterpret_cast<unsigned int>(&this[-0x00003FF8]) + rand());

        this->m_secdata->FakePtr1 = static_cast<unsigned char>(LOBYTE(this->FakePtr1));
        this->m_secdata->FakePtr2 = static_cast<unsigned char>(LOBYTE(this->FakePtr2));

        this->SetData(data);
    }

    ~TSecType()
    {
        if (this->m_secdata)
        {
            /* ZAllocEx<ZAllocAnonSelector>(0x017E3E1C)->Free(this->m_secdata); */
            free(this->m_secdata);
        }
    }

    T GetData()
    {
        T decrypted_data = 0;
        unsigned short checksum = 0;

        for (unsigned char i = 0, key = 0; i < (sizeof(T) + 1); i++)
        {
            key = (i == 0 ? this->m_secdata->bKey : reinterpret_cast<unsigned char*>(&this->m_secdata->data)[i - 1] + key + 42);

            if (i > 0)
            {
                checksum = (i > 1 ? ((8 * checksum) | (key + (checksum >> 13))) : ((key + 4) | 0xD328));
            }

            if (i < sizeof(T))
            {
                if (!key)
                {
                    key = 42;
                }

                reinterpret_cast<unsigned char*>(&decrypted_data)[i] = reinterpret_cast<unsigned char*>(&this->m_secdata->data)[i] ^ key;
            }
        }

        if ((this->m_secdata->wChecksum != checksum) || (LOBYTE(this->FakePtr1) != this->m_secdata->FakePtr1) || (LOBYTE(this->FakePtr2) != this->m_secdata->FakePtr2))
        {
            return 0;
        }

        return decrypted_data;
    }

    operator T() { return GetData(); }

    unsigned short SetData(T data)
    {
        this->m_secdata->bKey = static_cast<unsigned char>(rand());
        this->m_secdata->wChecksum = static_cast<unsigned short>(39525);

        for (unsigned char i = 0, key = 0; i < (sizeof(T) + 1); i++)
        {
            key = (i == 0 ? this->m_secdata->bKey : (key ^ reinterpret_cast<unsigned char*>(&data)[i - 1]) + key + 42);

            if (i > 0)
            {
                this->m_secdata->wChecksum = ((8 * this->m_secdata->wChecksum) | (key + (this->m_secdata->wChecksum >> 13)));
            }

            if (i < sizeof(T))
            {
                if (!key)
                {
                    key = 42;
                }

                reinterpret_cast<unsigned char*>(&this->m_secdata->data)[i] = reinterpret_cast<unsigned char*>(&data)[i] ^ key;
            }
        }

        return this->m_secdata->wChecksum;
    }

    void TSecType::operator=(T rhs) { SetData(rhs); }

private:
    unsigned int FakePtr1;
    unsigned int FakePtr2;
    TSecData<T>* m_secdata;
};

struct SECPOINT
{
    TSecType<long> y;
    TSecType<long> x;

    operator POINT() { return{ x, y }; }
};