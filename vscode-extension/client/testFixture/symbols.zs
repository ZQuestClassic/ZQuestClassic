/** COMMENT IGNORED */
// COMMENT IGNORED
int postfix = 1; // THIS COMMENT USED #1
int postfix2 = postfix; // THIS COMMENT USED #2
/** x marks the spot
 * still line one still line one still line one still line one still line one
 * 
 * line two
 * line two still
 *
 * line three
 *
 * - list item 1
 * - list item 2
 * - list item 3
*/
int x = 1;
// hello world
int y, z = 2;

// A car, duh
class Car {
    /** How fast it goes. See [vroom|the vroom method] or [speed] or [x] or [Waitframe] or [bitmap] or [bitmap::Width] */
    int speed;

    // a ctor with 1 param
    Car(int speed) {
        this->speed = speed;
    }

	/** Go fast */
    void vroom() {
        speed = 1;
    }

    void vroom2() {
        vroom();
    }
}

class IhaveAdefCtor {
}

// how fancy something looks
enum Brand {
    // top class
    Fancy,
    // absolute trash
    Dull,
    Mid // meh
};

// @Bitflags("int")
enum FilterBitflags
{
    F_SHINY, F_UGLY, F_SMALL, F_FAST
};

// filters with [FilterBitflags].
FilterBitflags do_filter(FilterBitflags f)
{
    Trace(f);
    return f;
}

Car make_car()
{
    Car c;
    return c;
}

// TODO: script decl node doesnt capture comments
// a ffc script called Metadata
ffc script Metadata {
    // run me
    void run(int radius, int speed, int angle, int radius2, int angle2) {
        printf("%d %d\n", postfix + postfix2, 1 + x + 2);
        utils::fn(postfix);
        do_filter(F_SHINY);
        // i am a car
        // [lololol|i dont exist]
        auto c = new Car(2);
        printf("%d %d %d\n", c->speed, radius, utils::hmm + y + z + Dull + Fancy + Mid);
        auto d = new IhaveAdefCtor();
        Car c2 = new Car(2);
        // equal to [c->speed]
        int e = c->speed;
        WrapDeg(e);
        // an array
        int arr[] = {1, 2, 3};
        for (v : arr) {
            Trace(v);
        }
        if (int ahhh = 1)
        {
            Trace(ahhh);
        }
        Brand b = Fancy;
        Trace(b);
    }
}

// a namespace for cool things
namespace utils {
    /** A lovely function. See [utils::fn] or [x] or [fn] or [Car] */
    void fn(int a) {

    }

    int hmm; // hmmmmmm See [utils::fn] or [x] or [fn]
}

bool T() {
	return true;
}

bool T2()
{
	bool b = true && T();
	return b;
}
