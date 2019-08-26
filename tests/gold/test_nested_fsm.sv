module mod (
  input logic  clk,
  input logic [1:0] in,
  output logic [1:0] out,
  input logic  rst
);

typedef enum logic[1:0] {
  Color_Blue = 2'h0,
  Color_Red = 2'h1,
  HSV_idle = 2'h2
} Color_state;
Color_state   Color_current_state;
Color_state   Color_next_state;

always @(posedge clk, posedge rst) begin
  if (rst) begin
    Color_current_state <= Color_Red;
  end
  else Color_current_state <= Color_next_state;
end
always_comb begin
  unique case (Color_current_state)
    Color_Blue: if (in == 2'h1) begin
      Color_next_state = Color_Red;
    end
    Color_Red: if (in == 2'h1) begin
      Color_next_state = Color_Blue;
    end
    else if (in == 2'h0) begin
      Color_next_state = Color_Red;
    end
    else if (in == 2'h2) begin
      Color_next_state = HSV_idle;
    end
    HSV_idle: if (in == 2'h0) begin
      Color_next_state = Color_Red;
    end
    default: begin end
  endcase
end
always_comb begin
  unique case (Color_current_state)
    Color_Blue: begin :Color_Color_Blue_Output
        out = 2'h1;
      end :Color_Color_Blue_Output
    Color_Red: begin :Color_Color_Red_Output
        out = 2'h2;
      end :Color_Color_Red_Output
    HSV_idle: begin :Color_HSV_idle_Output
        out = 2'h2;
      end :Color_HSV_idle_Output
    default: begin end
  endcase
end
endmodule   // mod
