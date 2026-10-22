/*
** $Id: llex.c $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#define llex_c
#define LUA_CORE

#include "lprefix.h"


#include <locale.h>
#include <string.h>

#include "lua.h"

#include "lctype.h"
#include "ldebug.h"
#include "ldo.h"
#include "lgc.h"
#include "llex.h"
#include "lobject.h"
#include "lparser.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "lzio.h"



#define next(ls)	(ls->current = zgetc(ls->z))


/* minimum size for string buffer */
#만약 !defined(LUA_MINBUFFER)
#define LUA_MINBUFFER   32
#endif


#define currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')


/* ORDER RESERVED */
static const char *const luaX_tokens [] = {
    "and", "break", "do", "else", "elseif",
    "end", "false", "for", "function", "global", "goto", "if",
    "in", "local", "nil", "not", "or", "repeat",
    "return", "then", "true", "until", "while",
    "//", "..", "...", "==", ">=", "<=", "~=",
    "<<", ">>", "::", "<eof>",
    "<number>", "<integer>", "<name>", "<string>"
};


#define save_and_next(ls) (save(ls, ls->current), next(ls))


static l_noret lexerror (LexState *ls, const char *msg, 정수 token);


static void save (LexState *ls, 정수 c) {
  Mbuffer *b = ls->buff;
  만약 (luaZ_bufflen(b) + 1 > luaZ_sizebuffer(b)) {
    size_t newsize = luaZ_sizebuffer(b);  /* get old size */;
    만약 (newsize >= (MAX_SIZE/3 * 2))  /* larger than MAX_SIZE/1.5 ? */
      lexerror(ls, "lexical element too long", 0);
    newsize += (newsize >> 1);  /* new size is 1.5 times the old one */
    luaZ_resizebuffer(ls->L, b, newsize);
  }
  b->buffer[luaZ_bufflen(b)++] = cast_char(c);
}


void luaX_init (lua_State *L) {
  정수 i;
  TString *e = luaS_newliteral(L, LUA_ENV);  /* create env name */
  luaC_fix(L, obj2gco(e));  /* never collect this name */
  위해 (i=0; i<NUM_RESERVED; i++) {
    TString *ts = luaS_new(L, luaX_tokens[i]);
    luaC_fix(L, obj2gco(ts));  /* reserved words are never collected */
    ts->extra = cast_byte(i+1);  /* reserved word */
  }
}


const char *luaX_token2str (LexState *ls, 정수 token) {
  만약 (token < FIRST_RESERVED) {  /* single-byte symbols? */
    만약 (lisprint(token))
      반환 luaO_pushfstring(ls->L, "'%c'", token);
    아니면  /* control character */
      반환 luaO_pushfstring(ls->L, "'<\\%d>'", token);
  }
  아니면 {
    const char *s = luaX_tokens[token - FIRST_RESERVED];
    만약 (token < TK_EOS)  /* fixed format (symbols and reserved words)? */
      반환 luaO_pushfstring(ls->L, "'%s'", s);
    아니면  /* names, strings, and numerals */
      반환 s;
  }
}


static const char *txtToken (LexState *ls, 정수 token) {
  switch (token) {
    case TK_NAME: case TK_STRING:
    case TK_FLT: case TK_INT:
      save(ls, '\0');
      반환 luaO_pushfstring(ls->L, "'%s'", luaZ_buffer(ls->buff));
    default:
      반환 luaX_token2str(ls, token);
  }
}


static l_noret lexerror (LexState *ls, const char *msg, 정수 token) {
  msg = luaG_addinfo(ls->L, msg, ls->source, ls->linenumber);
  만약 (token)
    luaO_pushfstring(ls->L, "%s near %s", msg, txtToken(ls, token));
  luaD_throw(ls->L, LUA_ERRSYNTAX);
}


l_noret luaX_syntaxerror (LexState *ls, const char *msg) {
  lexerror(ls, msg, ls->t.token);
}


/*
** Anchors a string in scanner's table so that it will not be collected
** until the end of the compilation; by that time it should be anchored
** somewhere. It also internalizes long strings, ensuring there is only
** one copy of each unique string.
*/
static TString *anchorstr (LexState *ls, TString *ts) {
  lua_State *L = ls->L;
  TValue oldts;
  정수 tag = luaH_getstr(ls->h, ts, &oldts);
  만약 (!tagisempty(tag))  /* string already present? */
    반환 tsvalue(&oldts);  /* use stored value */
  아니면 {  /* create a new entry */
    TValue *stv = s2v(L->top.p++);  /* reserve stack space for string */
    setsvalue(L, stv, ts);  /* push (anchor) the string on the stack */
    luaH_set(L, ls->h, stv, stv);  /* t[string] = string */
    /* table is not a metatable, so it does not need to invalidate cache */
    luaC_checkGC(L);
    L->top.p--;  /* remove string from stack */
    반환 ts;
  }
}


/*
** Creates a new string and anchors it in scanner's table.
*/
TString *luaX_newstring (LexState *ls, const char *str, size_t l) {
  반환 anchorstr(ls, luaS_newlstr(ls->L, str, l));
}


/*
** increment line number and skips newline sequence (any of
** \n, \r, \n\r, or \r\n)
*/
static void inclinenumber (LexState *ls) {
  정수 old = ls->current;
  lua_assert(currIsNewline(ls));
  next(ls);  /* skip '\n' or '\r' */
  만약 (currIsNewline(ls) && ls->current != old)
    next(ls);  /* skip '\n\r' or '\r\n' */
  만약 (++ls->linenumber >= INT_MAX)
    lexerror(ls, "chunk has too many lines", 0);
}


void luaX_setinput (lua_State *L, LexState *ls, ZIO *z, TString *source,
                    정수 firstchar) {
  ls->t.token = 0;
  ls->L = L;
  ls->current = firstchar;
  ls->lookahead.token = TK_EOS;  /* no look-ahead token */
  ls->z = z;
  ls->fs = NULL;
  ls->linenumber = 1;
  ls->lastline = 1;
  ls->source = source;
  /* all three strings here ("_ENV", "break", "global") were fixed,
     so they cannot be collected */
  ls->envn = luaS_newliteral(L, LUA_ENV);  /* get env string */
  ls->brkn = luaS_newliteral(L, "break");  /* get "break" string */
#만약 LUA_COMPAT_GLOBAL
  /* compatibility mode: "global" is not a reserved word */
  ls->glbn = luaS_newliteral(L, "global");  /* get "global" string */
  ls->glbn->extra = 0;  /* mark it as not reserved */
#endif
  luaZ_resizebuffer(ls->L, ls->buff, LUA_MINBUFFER);  /* initialize buffer */
}



/*
** =======================================================
** LEXICAL ANALYZER
** =======================================================
*/


static 정수 check_next1 (LexState *ls, 정수 c) {
  만약 (ls->current == c) {
    next(ls);
    반환 1;
  }
  아니면 반환 0;
}


/*
** Check whether current char is in set 'set' (with two chars) and
** saves it
*/
static 정수 check_next2 (LexState *ls, const char *set) {
  lua_assert(set[2] == '\0');
  만약 (ls->current == set[0] || ls->current == set[1]) {
    save_and_next(ls);
    반환 1;
  }
  아니면 반환 0;
}


/* LUA_NUMBER */
/*
** This function is quite liberal in what it accepts, as 'luaO_str2num'
** will reject ill-formed numerals. Roughly, it accepts the following
** pattern:
**
**   %d(%x|%.|([Ee][+-]?))* | 0[Xx](%x|%.|([Pp][+-]?))*
**
** The only tricky part is to accept [+-] only after a valid exponent
** mark, to avoid reading '3-4' or '0xe+1' as a single number.
**
** The caller might have already read an initial dot.
*/
static 정수 read_numeral (LexState *ls, SemInfo *seminfo) {
  TValue obj;
  const char *expo = "Ee";
  정수 first = ls->current;
  lua_assert(lisdigit(ls->current));
  save_and_next(ls);
  만약 (first == '0' && check_next2(ls, "xX"))  /* hexadecimal? */
    expo = "Pp";
  위해 (;;) {
    만약 (check_next2(ls, expo))  /* exponent mark? */
      check_next2(ls, "-+");  /* optional exponent sign */
    아니면 만약 (lisxdigit(ls->current) || ls->current == '.')  /* '%x|%.' */
      save_and_next(ls);
    아니면 break;
  }
  만약 (lislalpha(ls->current))  /* is numeral touching a letter? */
    save_and_next(ls);  /* force an error */
  save(ls, '\0');
  만약 (luaO_str2num(luaZ_buffer(ls->buff), &obj) == 0)  /* format error? */
    lexerror(ls, "malformed number", TK_FLT);
  만약 (ttisinteger(&obj)) {
    seminfo->i = ivalue(&obj);
    반환 TK_INT;
  }
  아니면 {
    lua_assert(ttisfloat(&obj));
    seminfo->r = fltvalue(&obj);
    반환 TK_FLT;
  }
}


/*
** read a sequence '[=*[' or ']=*]', leaving the last bracket. If
** sequence is well formed, return its number of '='s + 2; otherwise,
** return 1 if it is a single bracket (no '='s and no 2nd bracket);
** otherwise (an unfinished '[==...') return 0.
*/
static size_t skip_sep (LexState *ls) {
  size_t count = 0;
  정수 s = ls->current;
  lua_assert(s == '[' || s == ']');
  save_and_next(ls);
  동안 (ls->current == '=') {
    save_and_next(ls);
    count++;
  }
  반환 (ls->current == s) ? count + 2
         : (count == 0) ? 1
         : 0;
}


static void read_long_string (LexState *ls, SemInfo *seminfo, size_t sep) {
  정수 line = ls->linenumber;  /* initial line (for error message) */
  save_and_next(ls);  /* skip 2nd '[' */
  만약 (currIsNewline(ls))  /* string starts with a newline? */
    inclinenumber(ls);  /* skip it */
  위해 (;;) {
    switch (ls->current) {
      case EOZ: {  /* error */
        const char *what = (seminfo ? "string" : "comment");
        const char *msg = luaO_pushfstring(ls->L,
                     "unfinished long %s (starting at line %d)", what, line);
        lexerror(ls, msg, TK_EOS);
        break;  /* to avoid warnings */
      }
      case ']': {
        만약 (skip_sep(ls) == sep) {
          save_and_next(ls);  /* skip 2nd ']' */
          goto endloop;
        }
        break;
      }
      case '\n': case '\r': {
        save(ls, '\n');
        inclinenumber(ls);
        만약 (!seminfo) luaZ_resetbuffer(ls->buff);  /* avoid wasting space */
        break;
      }
      default: {
        만약 (seminfo) save_and_next(ls);
        아니면 next(ls);
      }
    }
  } endloop:
  만약 (seminfo)
    seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + sep,
                                     luaZ_bufflen(ls->buff) - 2 * sep);
}


static void esccheck (LexState *ls, 정수 c, const char *msg) {
  만약 (!c) {
    만약 (ls->current != EOZ)
      save_and_next(ls);  /* add current to buffer for error message */
    lexerror(ls, msg, TK_STRING);
  }
}


static 정수 gethexa (LexState *ls) {
  save_and_next(ls);
  esccheck (ls, lisxdigit(ls->current), "hexadecimal digit expected");
  반환 luaO_hexavalue(ls->current);
}


static 정수 readhexaesc (LexState *ls) {
  정수 r = gethexa(ls);
  r = (r << 4) + gethexa(ls);
  luaZ_buffremove(ls->buff, 2);  /* remove saved chars from buffer */
  반환 r;
}


/*
** When reading a UTF-8 escape sequence, save everything to the buffer
** for error reporting in case of errors; 'i' counts the number of
** saved characters, so that they can be removed if case of success.
*/
static l_uint32 readutf8esc (LexState *ls) {
  l_uint32 r;
  정수 i = 4;  /* number of chars to be removed: start with #"\u{X" */
  save_and_next(ls);  /* skip 'u' */
  esccheck(ls, ls->current == '{', "missing '{'");
  r = cast_uint(gethexa(ls));  /* must have at least one digit */
  동안 (cast_void(save_and_next(ls)), lisxdigit(ls->current)) {
    i++;
    esccheck(ls, r <= (0x7FFFFFFFu >> 4), "UTF-8 value too large");
    r = (r << 4) + luaO_hexavalue(ls->current);
  }
  esccheck(ls, ls->current == '}', "missing '}'");
  next(ls);  /* skip '}' */
  luaZ_buffremove(ls->buff, i);  /* remove saved chars from buffer */
  반환 r;
}


static void utf8esc (LexState *ls) {
  char buff[UTF8BUFFSZ];
  정수 n = luaO_utf8esc(buff, readutf8esc(ls));
  위해 (; n > 0; n--)  /* add 'buff' to string */
    save(ls, buff[UTF8BUFFSZ - n]);
}


static 정수 readdecesc (LexState *ls) {
  정수 i;
  정수 r = 0;  /* result accumulator */
  위해 (i = 0; i < 3 && lisdigit(ls->current); i++) {  /* read up to 3 digits */
    r = 10*r + ls->current - '0';
    save_and_next(ls);
  }
  esccheck(ls, r <= UCHAR_MAX, "decimal escape too large");
  luaZ_buffremove(ls->buff, i);  /* remove read digits from buffer */
  반환 r;
}


static void read_string (LexState *ls, 정수 del, SemInfo *seminfo) {
  save_and_next(ls);  /* keep delimiter (for error messages) */
  동안 (ls->current != del) {
    switch (ls->current) {
      case EOZ:
        lexerror(ls, "unfinished string", TK_EOS);
        break;  /* to avoid warnings */
      case '\n':
      case '\r':
        lexerror(ls, "unfinished string", TK_STRING);
        break;  /* to avoid warnings */
      case '\\': {  /* escape sequences */
        정수 c;  /* final character to be saved */
        save_and_next(ls);  /* keep '\\' for error messages */
        switch (ls->current) {
          case 'a': c = '\a'; goto read_save;
          case 'b': c = '\b'; goto read_save;
          case 'f': c = '\f'; goto read_save;
          case 'n': c = '\n'; goto read_save;
          case 'r': c = '\r'; goto read_save;
          case 't': c = '\t'; goto read_save;
          case 'v': c = '\v'; goto read_save;
          case 'x': c = readhexaesc(ls); goto read_save;
          case 'u': utf8esc(ls);  goto no_save;
          case '\n': case '\r':
            inclinenumber(ls); c = '\n'; goto only_save;
          case '\\': case '\"': case '\'':
            c = ls->current; goto read_save;
          case EOZ: goto no_save;  /* will raise an error next loop */
          case 'z': {  /* zap following span of spaces */
            luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
            next(ls);  /* skip the 'z' */
            동안 (lisspace(ls->current)) {
              만약 (currIsNewline(ls)) inclinenumber(ls);
              아니면 next(ls);
            }
            goto no_save;
          }
          default: {
            esccheck(ls, lisdigit(ls->current), "invalid escape sequence");
            c = readdecesc(ls);  /* digital escape '\ddd' */
            goto only_save;
          }
        }
       read_save:
         next(ls);
         /* go through */
       only_save:
         luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
         save(ls, c);
         /* go through */
       no_save: break;
      }
      default:
        save_and_next(ls);
    }
  }
  save_and_next(ls);  /* skip delimiter */
  seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + 1,
                                   luaZ_bufflen(ls->buff) - 2);
}


static 정수 llex (LexState *ls, SemInfo *seminfo) {
  luaZ_resetbuffer(ls->buff);
  위해 (;;) {
    switch (ls->current) {
      case '\n': case '\r': {  /* line breaks */
        inclinenumber(ls);
        break;
      }
      case ' ': case '\f': case '\t': case '\v': {  /* spaces */
        next(ls);
        break;
      }
      case '-': {  /* '-' or '--' (comment) */
        next(ls);
        만약 (ls->current != '-') 반환 '-';
        /* else is a comment */
        next(ls);
        만약 (ls->current == '[') {  /* long comment? */
          size_t sep = skip_sep(ls);
          luaZ_resetbuffer(ls->buff);  /* 'skip_sep' may dirty the buffer */
          만약 (sep >= 2) {
            read_long_string(ls, NULL, sep);  /* skip long comment */
            luaZ_resetbuffer(ls->buff);  /* previous call may dirty the buff. */
            break;
          }
        }
        /* else short comment */
        동안 (!currIsNewline(ls) && ls->current != EOZ)
          next(ls);  /* skip until end of line (or end of file) */
        break;
      }
      case '[': {  /* long string or simply '[' */
        size_t sep = skip_sep(ls);
        만약 (sep >= 2) {
          read_long_string(ls, seminfo, sep);
          반환 TK_STRING;
        }
        아니면 만약 (sep == 0)  /* '[=...' missing second bracket? */
          lexerror(ls, "invalid long string delimiter", TK_STRING);
        반환 '[';
      }
      case '=': {
        next(ls);
        만약 (check_next1(ls, '=')) 반환 TK_EQ;  /* '==' */
        아니면 반환 '=';
      }
      case '<': {
        next(ls);
        만약 (check_next1(ls, '=')) 반환 TK_LE;  /* '<=' */
        아니면 만약 (check_next1(ls, '<')) 반환 TK_SHL;  /* '<<' */
        아니면 반환 '<';
      }
      case '>': {
        next(ls);
        만약 (check_next1(ls, '=')) 반환 TK_GE;  /* '>=' */
        아니면 만약 (check_next1(ls, '>')) 반환 TK_SHR;  /* '>>' */
        아니면 반환 '>';
      }
      case '/': {
        next(ls);
        만약 (check_next1(ls, '/')) 반환 TK_IDIV;  /* '//' */
        아니면 반환 '/';
      }
      case '~': {
        next(ls);
        만약 (check_next1(ls, '=')) 반환 TK_NE;  /* '~=' */
        아니면 반환 '~';
      }
      case ':': {
        next(ls);
        만약 (check_next1(ls, ':')) 반환 TK_DBCOLON;  /* '::' */
        아니면 반환 ':';
      }
      case '"': case '\'': {  /* short literal strings */
        read_string(ls, ls->current, seminfo);
        반환 TK_STRING;
      }
      case '.': {  /* '.', '..', '...', or number */
        save_and_next(ls);
        만약 (check_next1(ls, '.')) {
          만약 (check_next1(ls, '.'))
            반환 TK_DOTS;   /* '...' */
          아니면 반환 TK_CONCAT;   /* '..' */
        }
        아니면 만약 (!lisdigit(ls->current)) 반환 '.';
        아니면 반환 read_numeral(ls, seminfo);
      }
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9': {
        반환 read_numeral(ls, seminfo);
      }
      case EOZ: {
        반환 TK_EOS;
      }
      default: {
        만약 (lislalpha(ls->current)) {  /* identifier or reserved word? */
          TString *ts;
          do {
            save_and_next(ls);
          } 동안 (lislalnum(ls->current));
          /* find or create string */
          ts = luaS_newlstr(ls->L, luaZ_buffer(ls->buff),
                                   luaZ_bufflen(ls->buff));
          만약 (isreserved(ts))   /* reserved word? */
            반환 ts->extra - 1 + FIRST_RESERVED;
          아니면 {
            seminfo->ts = anchorstr(ls, ts);
            반환 TK_NAME;
          }
        }
        아니면 {  /* single-char tokens ('+', '*', '%', '{', '}', ...) */
          정수 c = ls->current;
          next(ls);
          반환 c;
        }
      }
    }
  }
}


void luaX_next (LexState *ls) {
  ls->lastline = ls->linenumber;
  만약 (ls->lookahead.token != TK_EOS) {  /* is there a look-ahead token? */
    ls->t = ls->lookahead;  /* use this one */
    ls->lookahead.token = TK_EOS;  /* and discharge it */
  }
  아니면
    ls->t.token = llex(ls, &ls->t.seminfo);  /* read next token */
}


정수 luaX_lookahead (LexState *ls) {
  lua_assert(ls->lookahead.token == TK_EOS);
  ls->lookahead.token = llex(ls, &ls->lookahead.seminfo);
  반환 ls->lookahead.token;
}

