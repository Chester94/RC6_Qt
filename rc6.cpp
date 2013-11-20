#include "rc6.h"

RC6::RC6()
{
    key = "0123456789012345";

    w = 32;
    r = 20;
    l = 16;

    round_key = new RoundKey(w, r, l, key);
}

RC6::RC6(int _w, int _r, int _l, QString _key)
{
    key = _key.toStdString();

    w = _w;
    r = _r;
    l = _l;

    round_key = new RoundKey(w, r, l, key);
}

void RC6::Encryption()
{
    QFile* f1 = new QFile("D:\\test.jpg");
    f1->open(QIODevice::ReadOnly);
    QDataStream in(f1);

    QFile* f2 = new QFile("D:\\test_e.jpg");
    f2->open(QIODevice::WriteOnly);
    QDataStream out(f2);

    round_key->Initialization(key);

    while( ReadBlock(in) )
    {
        EncryptionBlock();
        WriteBlock(out);
    }

    f1->close();
    f2->close();
}

void RC6::Decryption()
{
    QFile* f1 = new QFile("D:\\test_e.jpg");
    f1->open(QIODevice::ReadOnly);
    QDataStream in(f1);

    QFile* f2 = new QFile("D:\\test_d.jpg");
    f2->open(QIODevice::WriteOnly);
    QDataStream out(f2);

    round_key->Initialization(key);

    while( ReadBlock(in) )
    {
        DecryptionBlock();
        WriteBlock(out);
    }

    f1->close();
    f2->close();
}

void RC6::EncryptionBlock()
{
    quint64  t, u, logw = qLn(w)/qLn(2);

    round_key->Advanced();

    block[1] = mod(block[1] + (*round_key)[0]);
    block[3] = mod(block[3] + (*round_key)[1]);

    for( int i = 1; i < r+1; i++ )
    {
        t = BCS::Shift(mod( block[1] * ( 2 * block[1] + 1 ) ), w, logw, toLeft);
        u = BCS::Shift(mod( block[3] * ( 2 * block[3] + 1 ) ), w, logw, toLeft);

        block[0] = mod(BCS::Shift(block[0] xor t, w, u, toLeft) + (*round_key)[2*i]);
        block[2] = mod(BCS::Shift(block[2] xor u, w, t, toLeft) + (*round_key)[2*i + 1]);

        block.ShiftLeft();
    }

    block[0] = mod(block[0] + (*round_key)[2*r + 2]);
    block[2] = mod(block[2] + (*round_key)[2*r + 3]);
}

void RC6::DecryptionBlock()
{
    quint64  t, u, logw = qLn(w)/qLn(2);

    round_key->Advanced();

    block[2] = mod(block[2] - (*round_key)[2*r +3]);
    block[0] = mod(block[0] - (*round_key)[2*r +2]);

    for( int i = r; i > 0; i-- )
    {
        block.ShiftRight();

        t = BCS::Shift(mod(block[1] * ( 2 * block[1] + 1 ) ), w, logw, toLeft);
        u = BCS::Shift(mod(block[3] * ( 2 * block[3] + 1 ) ), w, logw, toLeft);

        block[0] = BCS::Shift(mod(block[0] - (*round_key)[2*i]), w, u, toRight) xor t;
        block[2] = BCS::Shift(mod(block[2] - (*round_key)[2*i + 1]), w, t, toRight) xor u;
    }

    block[3] = mod(block[3] - (*round_key)[1]);
    block[1] = mod(block[1] - (*round_key)[0]);
}

int RC6::ReadBlock(QDataStream &in)
{
    block.set(0, 0, 0, 0);

    if( in.status() == QDataStream::ReadPastEnd ) return 0;

    quint8 byte;
    bool f = false;

    int i, j;

    for( i = 0; i < 4; i++ )
    {
        for( j = 0; j < w/8; j++)
        {
            block[i] = block[i] << 8;

            if( ! ( in.status() == QDataStream::ReadPastEnd) )
                in >> byte;

            if( ! ( in.status() == QDataStream::ReadPastEnd) )
            {
                block[i] |= byte;
                f = true;
            }
        }

        if( in.status() == QDataStream::ReadPastEnd )
            if(f)
                return 1;
            else
                return 0;
    }

    if(f)
        return 1;
    else
        return 0;
}

void RC6::WriteBlock(QDataStream &out)
{
    for( int i = 0; i < 4; i++ )
        switch (w)
        {
            case 16 :
                out << (quint16)block[i];
                break;

            case 32 :
                out << (quint32)block[i];
                break;

            case 64 : default :
                out << (quint64)block[i];
                break;
        }
}


quint64 RC6::mod(quint64 number)
{
    if( w == 64)
        return (quint64)number;

    quint64 pow = qPow(2, w);

    return number % pow;
}
