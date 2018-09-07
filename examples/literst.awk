BEGIN {
   block=0;
   printf(".. highlight:: c\n\n");
}

{
   spos=match($0,"^ *[/][*][ ]?(.*)", x);
   mpos=match($0,"^ *[*][ ]?(.*)|^ *[*]$", y);
   epos=match($0,"^ *[*][/](.*)", z);

   if(spos > 0 && epos ==0)
   {
      printf("%s\n", x[1]);
      block=0;
   }
   else if ( spos == 0 && epos > 0)
   {
      printf("%s\n",z[1]);
      block=1;
   }
   else if(mpos > 0)
   {
      printf("%s\n",y[1]);
      block=0;
   }
   else if (epos > 0)
   {
      block=1;
   }
   else if ( epos == 0 && mpos == 0 && spos == 0 )
   {
      if (block==1)
      {
          block = 0;
          printf("\n::\n\n");
      }
      printf("    %s\n",$0);
   }
}

END {
   if(found==1)
   {
       print "there is unmatched comment"
   }
}
