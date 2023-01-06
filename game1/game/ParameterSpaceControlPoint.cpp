/*
 * Modification History
 *
 * 2004-August-9   Jason Rohrer
 * Created.
 *
 * 2004-August-12   Jason Rohrer
 * Optimized blendColorArrays function.
 */



#include "ParameterSpaceControlPoint.h"



#include <math.h>
        

        
ParameterSpaceControlPoint::~ParameterSpaceControlPoint() {

    // does nothing... here to make compilers happy

    }



Vector3D **ParameterSpaceControlPoint::blendVertexArrays(
    Vector3D **inFirstArray,
    int inFirstArrayLength,
    double inWeightFirstArray,
    Vector3D **inSecondArray,
    int inSecondArrayLength,
    int *outResultLength ) {

    
    
    double weightOfSecondArray = 1 - inWeightFirstArray;

    // blend has the same number of elements as the larger control array
    int resultLength = inFirstArrayLength;

    if( inSecondArrayLength > resultLength ) {
        resultLength = inSecondArrayLength;
        }

    Vector3D **blendVertices = new Vector3D*[ resultLength ];
    

    // map the larger array nto the smaller array to
    // blend
    int sizeLargerArray;
    int sizeSmallerArray;

    Vector3D **arrayWithMoreVertices;
    Vector3D **arrayWithFewerVertices;
    
    double weightOfLargerSet;
    double weightOfSmallerSet;
    

    if( inFirstArrayLength > inSecondArrayLength ) {
        sizeLargerArray = inFirstArrayLength;
        sizeSmallerArray = inSecondArrayLength;
        
        arrayWithMoreVertices = inFirstArray;
        arrayWithFewerVertices = inSecondArray;
        weightOfLargerSet = inWeightFirstArray;
        weightOfSmallerSet = weightOfSecondArray;

        
        }
    else {
        sizeLargerArray = inSecondArrayLength;
        sizeSmallerArray = inFirstArrayLength;

        arrayWithMoreVertices = inSecondArray;
        arrayWithFewerVertices = inFirstArray;
        weightOfLargerSet = weightOfSecondArray;
        weightOfSmallerSet = inWeightFirstArray;
        }


    // size of blend array is same as size of larger set
    
    // factor to map large array indices into the smaller array
    double mapFactor =
        (double)( sizeSmallerArray - 1 ) / (double)(sizeLargerArray - 1 );
    
    for( int i=0; i<resultLength; i++ ) {

        // find the index of our blend partner vertex in the smaller set
        int partnerIndex =
            (int)rint( i * mapFactor );
        
        
        Vector3D *largerSetVertex =
            arrayWithMoreVertices[i];
        Vector3D *smallerSetVertex = arrayWithFewerVertices[ partnerIndex ];

        blendVertices[i] =
            Vector3D::linearSum( largerSetVertex,
                                 smallerSetVertex,
                                 weightOfLargerSet );
        }


    *outResultLength = resultLength;

    return blendVertices;
    }

