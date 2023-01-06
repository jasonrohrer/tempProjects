/*
 * Modification History
 *
 * 2004-June-16   Jason Rohrer
 * Created.
 *
 * 2004-June-22   Jason Rohrer
 * Fixed a memory leak.
 *
 * 2004-August-15   Jason Rohrer
 * Added sound support.
 *
 * 2004-August-19   Jason Rohrer
 * Fixed a memory leak.
 *
 * 2004-August-20   Jason Rohrer
 * Added sound priorities.
 *
 * 2004-August-30   Jason Rohrer
 * Added bullet scaling parameters.
 */



#include "ShipBulletManager.h"



ShipBulletManager::ShipBulletManager( ShipBullet *inBulletTemplate,
                                      double inBulletScale,
                                      SoundPlayer *inPlayer,
                                      BulletSound *inBulletSoundTemplate,
                                      double inWorldWidth,
                                      double inWorldHeight )
    : mBulletTemplate( inBulletTemplate ),
      mBulletScale( inBulletScale ),
      mSoundPlayer( inPlayer ),
      mBulletSoundTemplate( inBulletSoundTemplate ),
      mCloseRangeParameters( new SimpleVector<double>() ),
      mFarRangeParameters( new SimpleVector<double>() ),
      mPowerModifiers( new SimpleVector<double>() ),
      mCurrentPowers( new SimpleVector<double>() ),
      mRangesInScreenUnits( new SimpleVector<double>() ),
      mRangeFractions( new SimpleVector<double>() ),
      mTimePassed( new SimpleVector<double>() ),
      mStartingPositions( new SimpleVector<Vector3D*>() ),
      mVelocitiesInScreenUnitsPerSecond( new SimpleVector<Vector3D*>() ),
      mCurrentPositions( new SimpleVector<Vector3D*>() ),
      mCurrentRotations( new SimpleVector<Angle3D*>() ),
      mCurrentRotationRates( new SimpleVector<double>() ),
      mSholdBeDestroyedFlags( new SimpleVector<char>() ) {

    mMaxXPosition = inWorldWidth / 2;
    mMinXPosition = -mMaxXPosition;

    mMaxYPosition = inWorldHeight / 2;
    mMinYPosition = -mMaxYPosition;
    
    }


        
ShipBulletManager::~ShipBulletManager() {
    delete mBulletTemplate;
    delete mBulletSoundTemplate;

    delete mCloseRangeParameters;
    delete mFarRangeParameters;
    delete mPowerModifiers;
    delete mCurrentPowers;
    
    delete mRangesInScreenUnits;
    delete mRangeFractions;
    delete mTimePassed;
    
    int numBullets = mStartingPositions->size();

    for( int i=0; i<numBullets; i++ ) {

        delete *( mStartingPositions->getElement( i ) );
        delete *( mVelocitiesInScreenUnitsPerSecond->getElement( i ) );
        delete *( mCurrentPositions->getElement( i ) );
        delete *( mCurrentRotations->getElement( i ) );
        }

    
    delete mStartingPositions;
    delete mVelocitiesInScreenUnitsPerSecond;
    delete mCurrentPositions;
    delete mCurrentRotations;
    
    delete mCurrentRotationRates;
    delete mSholdBeDestroyedFlags;
    }



void ShipBulletManager::addBullet(
    double inCloseRangeParameter,
    double inFarRangeParameter,
    double inPowerModifier,
    double inRangeInScreenUnits,
    Vector3D *inStartingPosition,
    Angle3D *inStartingRotation,
    Vector3D *inVelocityInScreenUnitsPerSecond ) {

    mCloseRangeParameters->push_back( inCloseRangeParameter );
    mFarRangeParameters->push_back( inFarRangeParameter );
    mPowerModifiers->push_back( inPowerModifier );
    mCurrentPowers->push_back( 0 );
    
    mRangesInScreenUnits->push_back( inRangeInScreenUnits );
    mRangeFractions->push_back( 0 );
    mTimePassed->push_back( 0 );
    
    mStartingPositions->push_back( inStartingPosition );
    mCurrentRotations->push_back( inStartingRotation );
    mCurrentRotationRates->push_back( 0 );

    mVelocitiesInScreenUnitsPerSecond->push_back(
        inVelocityInScreenUnitsPerSecond );
    mCurrentPositions->push_back( new Vector3D( inStartingPosition ) );

    mSholdBeDestroyedFlags->push_back( false );

    // play the sound
    PlayableSound *sound = mBulletSoundTemplate->getPlayableSound(
        inCloseRangeParameter,
        inFarRangeParameter,
        mSoundPlayer->getSampleRate() );

    // shot sounds are low priority
    mSoundPlayer->playSoundNow( sound, false, inPowerModifier );

    delete sound;
    }



double ShipBulletManager::getBulletPowerInCircle( Vector3D *inCircleCenter,
                                                  double inCircleRadius ) {
    double powerSum = 0;
    
    int numBullets = mCloseRangeParameters->size();

    for( int i=0; i<numBullets; i++ ) {
        double currentRotationRate;
        double power;
        
        SimpleVector<DrawableObject *> *bulletObjects =
            mBulletTemplate->getDrawableObjects(
                *( mCloseRangeParameters->getElement( i ) ),
                *( mFarRangeParameters->getElement( i ) ),
                *( mRangeFractions->getElement( i ) ),
                &power,
                &currentRotationRate );


        int numObjects = bulletObjects->size();

        char alreadyHit = false;
        for( int j=0; j<numObjects && !alreadyHit; j++ ) {

            DrawableObject *currentObject =
                *( bulletObjects->getElement( j ) );

            if( !alreadyHit ) {
                // check if this part of bullet hits

                currentObject->scale( mBulletScale );
                currentObject->rotate(
                    *( mCurrentRotations->getElement( i ) ) );
                currentObject->move(
                    *( mCurrentPositions->getElement( i ) ) );

                if( currentObject->isBorderInCircle( inCircleCenter,
                                                     inCircleRadius ) ) {
                    powerSum += *( mCurrentPowers->getElement( i ) );
                    alreadyHit = true;
                    }
             
                }
            // otherwise, ignore this part of bullet
            }

        for( int j=0; j<numObjects; j++ ) {

            DrawableObject *currentObject =
                *( bulletObjects->getElement( j ) );
            
            delete currentObject;
            }

        delete bulletObjects;
        }

    return powerSum;
    }



void ShipBulletManager::passTime( double inTimeDeltaInSeconds ) {
    int numBullets = mStartingPositions->size();

    int i;
    for( i=0; i<numBullets; i++ ) {
        *( mTimePassed->getElement( i ) ) += inTimeDeltaInSeconds;

        Angle3D *currentRotation = *( mCurrentRotations->getElement( i ) );

        double rotationRate = *( mCurrentRotationRates->getElement( i ) );
        
        currentRotation->mZ += rotationRate * inTimeDeltaInSeconds;


        // compute new position
        Vector3D *newPosition =
            new Vector3D( *( mStartingPositions->getElement( i ) ) );

        Vector3D *travelVector = new Vector3D(
            *( mVelocitiesInScreenUnitsPerSecond->getElement( i ) ) );

        travelVector->scale( *( mTimePassed->getElement( i ) ) );

        newPosition->add( travelVector );

        delete *( mCurrentPositions->getElement( i ) );

        *( mCurrentPositions->getElement( i ) ) = newPosition;
        
        double distanceTraveled = travelVector->getLength();

        delete travelVector;


        // compute new range fraction

        double rangeLength = *( mRangesInScreenUnits->getElement( i ) );

        double newRangeFraction = distanceTraveled /  rangeLength;
        
        *( mRangeFractions->getElement( i ) ) = newRangeFraction;

        if( newRangeFraction >= 1 ) {
            // bullet has reached end of range
            // should be destroyed
            *( mSholdBeDestroyedFlags->getElement( i ) ) = true;
            }        
        }

    
    // walk through vectors and destroy bullets that are flagged
    // note that our loop range is adjusted as the vector length shrinks
    for( i=0; i<mCloseRangeParameters->size(); i++ ) {
        if( *( mSholdBeDestroyedFlags->getElement( i ) ) ) {

            mCloseRangeParameters->deleteElement( i );
            mFarRangeParameters->deleteElement( i );
            mPowerModifiers->deleteElement( i );

            mRangesInScreenUnits->deleteElement( i );
            mRangeFractions->deleteElement( i );

            mTimePassed->deleteElement( i );

            delete *( mStartingPositions->getElement( i ) );
            mStartingPositions->deleteElement( i );

            delete *( mCurrentRotations->getElement( i ) );
            mCurrentRotations->deleteElement( i );

            mCurrentRotationRates->deleteElement( i );

            delete *( mVelocitiesInScreenUnitsPerSecond->getElement( i ) );
            mVelocitiesInScreenUnitsPerSecond->deleteElement( i );

            delete *( mCurrentPositions->getElement( i ) );
            mCurrentPositions->deleteElement( i );

            mSholdBeDestroyedFlags->deleteElement( i );
            }
        }
    }



SimpleVector<DrawableObject*> *ShipBulletManager::getDrawableObjects() {

    SimpleVector<DrawableObject*> *returnVector =
        new SimpleVector<DrawableObject*>();
    
    int numBullets = mCloseRangeParameters->size();

    for( int i=0; i<numBullets; i++ ) {
        double power;
        double modifiedPower;
        double currentRotationRate;
        
        SimpleVector<DrawableObject *> *bulletObjects =
            mBulletTemplate->getDrawableObjects(
                *( mCloseRangeParameters->getElement( i ) ),
                *( mFarRangeParameters->getElement( i ) ),
                *( mRangeFractions->getElement( i ) ),
                &power,
                &currentRotationRate );

        double powerModifier = *( mPowerModifiers->getElement( i ) );
        
        modifiedPower = power * powerModifier;

        *( mCurrentPowers->getElement( i ) ) = modifiedPower;
        *( mCurrentRotationRates->getElement( i ) ) = currentRotationRate;

        int numObjects = bulletObjects->size();
        
        for( int j=0; j<numObjects; j++ ) {

            DrawableObject *currentObject =
                *( bulletObjects->getElement( j ) );

            currentObject->fade( powerModifier );

            currentObject->scale( mBulletScale );
            currentObject->rotate( *( mCurrentRotations->getElement( i ) ) );
            currentObject->move( *( mCurrentPositions->getElement( i ) ) );

            returnVector->push_back( currentObject );
            }

        delete bulletObjects;
        }

    return returnVector;
    }
