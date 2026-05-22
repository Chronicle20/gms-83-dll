#pragma once

class CAttrShoe : ZRefCounted {
    TSecType<double> mass;
    TSecType<double> walkAcc;
    TSecType<double> walkSpeed;
    TSecType<double> walkDrag;
    TSecType<double> walkSlant;
    TSecType<double> walkJump;
    TSecType<double> swimAcc;
    TSecType<double> swimSpeedH;
    TSecType<double> swimSpeedV;
    TSecType<double> flyAcc;
    TSecType<double> flySpeed;
};