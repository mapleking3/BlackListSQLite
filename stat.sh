#!/usr/bin/awk -f
BEGIN{
    sum=0;
    cnt=0;
    larget_cnt=0;
}
{
    if ($2 == "MIDDLE" && $5 == "us")
    {
        sum+=$4;
        cnt+=1;
        if ($4 > 1000)
        {
            printf("%d\n", $4);
            larget_cnt++;
        }
    }
}
END{
    printf("Avg:%.2f\n", sum/cnt);
    printf("Total:%d\n", larget_cnt);
}
