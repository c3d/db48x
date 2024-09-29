// ****************************************************************************
//  AppDelegate.mm                                               DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Basic iOS application infrastructure
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2024 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the terms outlined in LICENSE.txt
// ****************************************************************************
//   This file is part of DB48X.
//
//   DB48X is free software: you can redistribute it and/or modify
//   it under the terms outlined in the LICENSE.txt file
//
//   DB48X is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// ****************************************************************************

#import "AppDelegate.h"
#import "AppSettings.h"

#include "dmcp.h"
#include "tests.h"

@interface AppDelegate ()

@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application
    didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
// ----------------------------------------------------------------------------
//   Application launch
// ----------------------------------------------------------------------------
{
    // Load user settings
    NSUserDefaults *settings = [NSUserDefaults standardUserDefaults];
    theAppSettings.saveStateOnExit = ![settings boolForKey:@"DisableSaveState"];
    theAppSettings.saveStateWhenMovingToBackground = [settings boolForKey:@"EnableSaveStateWhenMovingToBackground"];
    theAppSettings.hapticFeedback = ![settings boolForKey:@"DisableHapticFeedback"];

    // Change directory to the app documents path
    NSString *bundle = [[NSBundle mainBundle] resourcePath];
    NSArray<NSString *> *documentsPath = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSLog(@"Document directories %@", documentsPath);
    NSString *documentsDirectory = [documentsPath lastObject];
    chdir([documentsDirectory cStringUsingEncoding:[NSString defaultCStringEncoding]]);
    NSLog(@"Running in directory %@", documentsDirectory);


    // Create the DB48X standard structure
    NSFileManager *fileManager = [NSFileManager defaultManager];
    for (NSString *subdir in @[@"state", @"help", @"help/img",@"config", @"data", @"library"])
    {
        NSString *directory = [documentsDirectory stringByAppendingPathComponent:subdir];
        NSError *error = nil;
        [fileManager createDirectoryAtPath:directory
               withIntermediateDirectories:YES
                                attributes:nil
                                     error:&error];
        if (error)
            NSLog(@"Error creating '%@' directory: %@", subdir, error);
    }

    // Copy initial files
    NSArray<NSString *> *filesToCopy = @[
        @"state/Demo.48S",
        @"state/Test.48S",
        @"config/equations.csv",
        @"config/library.csv",
        @"config/characters.csv",
        @"config/units.csv",
        @"config/constants.csv",
        @"library/Beeps.48s",
        @"library/CollatzBenchmark.48s",
        @"library/CollatzConjecture.48s",
        @"library/CountPrimes.48s",
        @"library/CurvePlottingExamples.48s",
        @"library/DrawingLines.48s",
        @"library/DrawingPatterns.48s",
        @"library/DrawingShapes.48s",
        @"library/DrawingText.48s",
        @"library/KineticEnergy.48s",
        @"library/NQueens.48s",
        @"library/RandomPlot.48s",
        @"library/RandomXYPlot.48s",
        @"library/Recursion.48s",
        @"library/SumTestWithFunction.48s",
        @"library/SumTestWithLoop.48s",
        @"library/TriangleEquations.48s",
        @"library/UnitsBenchmark.48s",
        @"library/WalkMan.48s",
        @"help/db50x.md",
        @"help/db48x.md",
        @"help/shifts.bmp",
        @"help/img/1-arithmetic-rpl.bmp",
        @"help/img/10-complex.bmp",
        @"help/img/1DElastiCollisions.bmp",
        @"help/img/2-arithmetic-cmd.bmp",
        @"help/img/3-fractions.bmp",
        @"help/img/4-basic-math.bmp",
        @"help/img/5-mixing-rpl-alg.bmp",
        @"help/img/6-trigonometrics.bmp",
        @"help/img/7-display-modes.bmp",
        @"help/img/8-help.bmp",
        @"help/img/9-angles.bmp",
        @"help/img/BernouilliEquation.bmp",
        @"help/img/BipolarTransistor.bmp",
        @"help/img/BlackBodyRadiation.bmp",
        @"help/img/BrewstersLaw.bmp",
        @"help/img/CantileverDeflection.bmp",
        @"help/img/CantileverMoment.bmp",
        @"help/img/CantileverShear.bmp",
        @"help/img/CantileverSlope.bmp",
        @"help/img/Circle.bmp",
        @"help/img/CircularRing.bmp",
        @"help/img/Conduction+Convection.bmp",
        @"help/img/Conduction.bmp",
        @"help/img/Cone.bmp",
        @"help/img/ConicalPendulum.bmp",
        @"help/img/Convection.bmp",
        @"help/img/CriticalAngle.bmp",
        @"help/img/CurrentDivider.bmp",
        @"help/img/Cylinder.bmp",
        @"help/img/CylindricalCapacitor.bmp",
        @"help/img/EccentricColumns.bmp",
        @"help/img/ElasticBuckling.bmp",
        @"help/img/Ellipse.bmp",
        @"help/img/FlowInFullPipes.bmp",
        @"help/img/FlowWIthLosses.bmp",
        @"help/img/ForceBetweenWires.bmp",
        @"help/img/HookesLaw.bmp",
        @"help/img/IsentropicFlow.bmp",
        @"help/img/JFETs.bmp",
        @"help/img/LawOfRefraction.bmp",
        @"help/img/MagneticFieldInSolenoid.bmp",
        @"help/img/MagneticFieldInToroid.bmp",
        @"help/img/MassSpringSystem.bmp",
        @"help/img/MohrsCircle.bmp",
        @"help/img/NMOSTransistor.bmp",
        @"help/img/NewtonGravitation.bmp",
        @"help/img/NormalStress.bmp",
        @"help/img/PNStepJunction.bmp",
        @"help/img/Parallelepiped.bmp",
        @"help/img/PlateCapacitor.bmp",
        @"help/img/PressureAtDepth.bmp",
        @"help/img/ProjectileMotion.bmp",
        @"help/img/RCTransient.bmp",
        @"help/img/RLCCurrentDelay.bmp",
        @"help/img/RLTransient.bmp",
        @"help/img/Rectangle.bmp",
        @"help/img/RegularPolygon.bmp",
        @"help/img/SeriesAndParallelC.bmp",
        @"help/img/SeriesAndParallelL.bmp",
        @"help/img/SeriesAndParallelR.bmp",
        @"help/img/ShearStress.bmp",
        @"help/img/SimpleDeflection.bmp",
        @"help/img/SimpleMoment.bmp",
        @"help/img/SimplePendulum.bmp",
        @"help/img/SimpleShear.bmp",
        @"help/img/SimpleSlope.bmp",
        @"help/img/SolenoidInductance.bmp",
        @"help/img/Sphere.bmp",
        @"help/img/SphericalReflection.bmp",
        @"help/img/SphericalRefraction.bmp",
        @"help/img/StraightWire.bmp",
        @"help/img/StressOnAnElement.bmp",
        @"help/img/ThermalExpansion.bmp",
        @"help/img/ThinLens.bmp",
        @"help/img/ToroidInductance.bmp",
        @"help/img/TorsionalPendulum.bmp",
        @"help/img/Triangle.bmp",
        @"help/img/VoltageDivider.bmp",
        @"help/img/bat-and-jet.bmp",
        @"help/img/command-line-and-catalog.bmp",
        @"help/img/definition_of_e.bmp",
        @"help/img/definition_of_pi.bmp",
        @"help/img/imaginary_unit.bmp",
        @"help/img/stack-and-menus.bmp",
    ];
    NSLog(@"Contents of resources: %@", [[fileManager enumeratorAtPath:bundle] allObjects]);
    NSLog(@"Contents of documents before copy: %@", [[fileManager enumeratorAtPath:documentsDirectory] allObjects]);
    for (NSString *file in filesToCopy)
    {
        NSString *source = [bundle stringByAppendingPathComponent:file];
        NSString *destination = [documentsDirectory stringByAppendingPathComponent:file];
        bool doCopy = ![fileManager fileExistsAtPath:destination];
        if (!doCopy)
        {
            NSDictionary* sdict = [fileManager attributesOfItemAtPath:source error:nil];
            NSDate* sdate = [sdict fileModificationDate];
            NSDictionary* ddict = [fileManager attributesOfItemAtPath:destination error:nil];
            NSDate* ddate = [ddict fileModificationDate];
            doCopy = [sdate compare:ddate] == NSOrderedDescending;
            if (doCopy)
            {
                [fileManager removeItemAtPath:destination error:nil];
            }
        }
        NSLog(@"%s copy %@", doCopy ? "Will" : "Won't", file);
        if (doCopy)
        {
            NSError *error = nil;
            [fileManager copyItemAtPath:source toPath:destination error:&error];
            if (error)
                NSLog(@"Error copying '%@': %@", file, error);
        }
    }
    NSLog(@"Contents of documents after copy: %@", [[fileManager enumeratorAtPath:documentsDirectory] allObjects]);

    // Dispatch the RPL thread
    dispatch_queue_t queue =
        dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0ul);
    dispatch_async(queue, ^{
            program_main();
        });

    return YES;
}


- (void)saveApplicationSettings
// ----------------------------------------------------------------------------
//   Save application settings to user defaults
// ----------------------------------------------------------------------------
{
    NSUserDefaults *settings = [NSUserDefaults standardUserDefaults];
    [settings setBool:!theAppSettings.saveStateOnExit
               forKey:@"DisableSaveState"];
    [settings setBool:theAppSettings.saveStateWhenMovingToBackground
               forKey:@"EnableSaveStateWhenMovingToBackground"];
    [settings setBool:!theAppSettings.hapticFeedback
               forKey:@"DisableHapticFeedback"];
}


- (void)applicationWillResignActive:(UIApplication *)application
// ----------------------------------------------------------------------------
//   Moving from active to inactive state
// ----------------------------------------------------------------------------
// Sent when the application is about to move from active to inactive  state.
// This can occur for certain types of temporary interruptions (such as
// an incoming phone call or SMS message) or when the user quits the application
// and it begins the transition to the background state.
//
// Use this method to pause ongoing tasks, disable timers, and throttle down
// OpenGL ES frame rates. Games should use this method to pause the game.
{
    [self saveApplicationSettings];
    if (theAppSettings.saveStateWhenMovingToBackground)
        key_push(tests::SAVE_PGM);
}


- (void)applicationDidEnterBackground:(UIApplication *)application
// ----------------------------------------------------------------------------
//   Entering background
// ----------------------------------------------------------------------------
// Use this method to release shared resources, save user data,
// invalidate timers, and store enough application state information to
// restore your application to its current state in case it is terminated later.
// If your application supports background execution, this method is called
// instead of applicationWillTerminate: when the user quits.
{
}


- (void)applicationWillEnterForeground:(UIApplication *)application
// ----------------------------------------------------------------------------
//   Transition from background to inactive state
// ----------------------------------------------------------------------------
// Here you can undo many of the changes made on entering the background.
{
}


- (void)applicationDidBecomeActive:(UIApplication *)application
// ----------------------------------------------------------------------------
//    Application becomes active
// ----------------------------------------------------------------------------
// Restart any tasks that were paused (or not yet started) while the
// application was inactive. If the application was previously in the
// background, optionally refresh the user interface.
{
}


- (void)applicationWillTerminate:(UIApplication *)application
// ----------------------------------------------------------------------------
//    Application is about to terminate
// ----------------------------------------------------------------------------
//   Save data if appropriate. See also applicationDidEnterBackground:.
{
    [self saveApplicationSettings];
    if (theAppSettings.saveStateOnExit)
        key_push(tests::SAVE_PGM);
}

- (BOOL)application:(UIApplication *)app openURL:(NSURL *)url options:(NSDictionary<UIApplicationOpenURLOptionsKey,id> *)options
// ----------------------------------------------------------------------------
//    Open external file
// ----------------------------------------------------------------------------
{
    NSString *path = [url path];
    NSLog(@"Opening URL %@ path %@", url, path);
    return NO;
}

@end
