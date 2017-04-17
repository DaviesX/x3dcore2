class ut_string extends String {
    constructor(s) {
        super(s);
    }
    last_after(stor) {
        var c = super.split(stor);
        return c[c.length - 1];
    }
}
//# sourceMappingURL=utils.js.map