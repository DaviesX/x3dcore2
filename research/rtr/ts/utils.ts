
class ut_string extends String
{
        constructor(s: string)
        {
                super(s);
        }

        public last_after(stor: string): string
        {
                var c = super.split(stor);
                return c[c.length - 1];
        }
}