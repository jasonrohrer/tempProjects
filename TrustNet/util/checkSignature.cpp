/*
 * Modification History
 *
 * 2004-November-14  Jason Rohrer
 * Created.
 */


#include "Primrose/common/CryptoUtils.h"

#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/stringUtils.h"

#include <stdio.h>
#include <string.h>


/**
 * Takes a hex-encoded public key as the first argument and a hex-encoded
 * signature as the second argument.
 *
 * If there are three arguments, then the third is used as the signed data.
 * If there are only two arguments, then the signed data is read from std in.
 *
 * Prints "OK" to std out if signature is correct.
 * Prints "FAILED" to std out if signature verification fails.
 */
int main( int inNumArgs, char **inArgs ) {

    if( inNumArgs != 3 &&
        inNumArgs != 4 ) {
        printf( "FAILED" );
        return 0;
        }

    char *publicKey = inArgs[1];
    char *signature = inArgs[2];

    int dataLength;
    unsigned char *readData;

    if( inNumArgs == 4 ) {
        // third argument is signed data
        dataLength = strlen( inArgs[3] );
        readData = (unsigned char *)( stringDuplicate( inArgs[3] ) );
        }
    else{
        // read signed data from std in
        SimpleVector<unsigned char> *readChars =
            new SimpleVector<unsigned char>();
    
        int readChar = getchar();
        while( readChar != EOF ) {
            readChars->push_back( (unsigned char)readChar );
            readChar = getchar();
            }


        dataLength = readChars->size();
        readData = readChars->getElementArray();
    
        delete readChars;
        }

    
    char signatureCorrect =
        CryptoUtils::rsaVerify( publicKey,
                                readData,
                                dataLength,
                                signature );

    delete [] readData;


    if( signatureCorrect ) {
        printf( "OK" );
        }
    else {
        printf( "FAILED" );
        }
    
    return 0;
    }
