int find_line_cross(double k1, double b1, double k2, double b2, struct PointD *cross_point);
int find_line_cross_inf(double k, double b, double x, struct PointD *cross_point);
void determine_line(double *k, double *b, struct PointD p1, struct PointD p2);
int cross_check_nfp(struct DxfFile curr_file, struct PointD offset, struct NfpPoint *head);
int out_of_nfp(struct DxfFile curr_file, struct NfpPoint *head, struct PointD offset, double width);
