var
  a: array[1..100010, 32..126] of int64;	//contains the edges between verteces, first index - vertex's number, second - the character through which the way passes
  suf, blue, b, par, vh: array[1..160010] of int64; //suf - suffix links of verteces; 
  													//blue - direct link through all unnecessary verteces to the vertex that contains the end of some word
  													//b - queue for verteces
  													//par - extra queue for parent verteces of the b queue 
  													//vh - extra queue that contains the letter that leads to vertex in the b queue
  flag: array[1..160010] of boolean; //the array which contains true on the verteces which are ends of the words, otherwise false
  i, j: longint;		//global variables for loop iterations
  ch: array[1..100010] of char;	//an array for inputting new words and some text
  n, m, h, l, r: int64; // n - number of words in the list to construct a trie; m - the length of considered word;
  						// h - the last vertex; l, r -pointers to the 1st and last elements in the queue

//procedure for pushing the new word into the trie 
procedure push(t, x: longint);

begin

  if x > m then 	//check: is it the end of added word, if yes we mark it in the array flag
    flag[t] := true
  else 
  begin
    
    if a[t][ord(ch[x])] <> 0 then    //check: if we already have the way through considered letter, we just go there
      push(a[t][ord(ch[x])], x + 1)
    else
    begin          					//else: if there is no way, we create new one by adding new vertex
      inc(h);
      a[t][ord(ch[x])] := h;
      push(h, x + 1);
    end;

  end;
end;

// procedure for making suffix link for some vertex
procedure make_suf(t, p, c: longint);
var
  x: longint;

begin
  x := suf[p];

  while a[x][c] = 0 do   //finding the appropriate vertex to put suffix link
    x := suf[x];

  x := a[x][c];
  suf[t] := x;

  if flag[x] then    	//additionally, beside suffix link to next element, we put link to the next end of the word vertex
    blue[t] := x
  else
    blue[t] := blue[x];
end;

//procedure for finding words in the text
procedure aho(m: longint);
var
  i, j, t, x: longint;

begin
  t := 1; 	//equal the pointer t to the first vertex

  for i := 1 to m do
  begin
    
    while a[t][ord(ch[i])] = 0 do 	//finding the way in the trie
      t := suf[t];
    
    t := a[t][ord(ch[i])];
    x := t;
    
    while x > 0 do     //check if the string contains some of given words or not
    begin
      
      if flag[x] then  //if yes we print the string
      begin
        
        for j := 1 to m do
          write(ch[j]);
        
        writeln;
        exit;
      end;

      x := blue[x];
    end;
  end;
end;

begin
  assign(input, 'input.txt'); reset(input);
  assign(output, 'output.txt'); rewrite(output);

  h := 1;

  readln(n);

  //Here we are entering the initial list of words to construct a trie
  for i := 1 to n do
  begin
    m := 0;

    while not eoln do
    begin
      inc(m);
      read(ch[m]);
    end;

    readln;
    push(1, 1);
  end;

  //Here we create extra vertex which will contain edges through all characters to the 1st vertex
  inc(h);

  for j := 32 to 126 do
    a[h][j] := 1;

  suf[1] := h;	// make the suffix link of 1st vertex to the extra vertex

  //Here by the way of BFS we construct suffix links to all existing verteces
  l := 1;
  r := 1;
  b[1] := 1;
  
  while l <= r do
  begin
    
    if l <> 1 then
      make_suf(b[l], par[l], vh[l]);
    
    for j := 32 to 126 do
      if a[b[l]][j] <> 0 then
      begin
        inc(r);
        b[r] := a[b[l]][j];
        par[r] := b[l];
        vh[r] := j;
      end;
    
    inc(l);
  end;

  // reading some list of strings
  while not eof do
  begin
    m := 0;
    
    while not eoln do
    begin
      inc(m);
      read(ch[m]);
    end;
    
    readln;

    aho(m); //finding the given words in the string
  end;

  close(output);
end.
