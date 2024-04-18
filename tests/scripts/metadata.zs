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
    /** How fast it goes. See {@link vroom|the vroom method} or {@link speed} or [x] */
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

// a ffc script called Metadata
ffc script Metadata {
    // run me
    void run(int radius, int speed, int angle, int radius2, int angle2) {
        printf("%d %d\n", postfix + postfix2, 1 + x + 2);
        utils::fn(postfix);
        // i am a car
        // {@link lololol|i dont exist}
        auto c = new Car(2);
        printf("%d %d %d\n", c->speed, radius, utils::hmm + y + z + Dull + Fancy + Mid);
        auto d = new IhaveAdefCtor();
        Car c2 = new Car(2); // TODO: let's make `Car` in type decl point to the class defn. also do same for auto?
    }
}

// a namespace for cool things
namespace utils {
    /** A lovely function. See {@link utils::fn} or {@link x} or {@link fn} or {@link Car} */
    void fn(int a) {

    }

    int hmm; // hmmmmmm See {@link utils::fn} or {@link x} or {@link fn}
}
