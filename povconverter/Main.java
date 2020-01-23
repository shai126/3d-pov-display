
import java.awt.AlphaComposite;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import javax.imageio.ImageIO;

/**
 * POV Converter
 * Converts images / font images to C code for use in persistence of vision
 * display project
 * @author Shai Schechter @shaisc
 */
public class Main {
    private enum Mode {
        FONT, IMAGE
    };
    private static final int NO_OF_ROWS = 64;
    private static final int NO_OF_COLS = 80;

    public static void main(String[] args) {
        try {
            Mode mode = Mode.valueOf(args[0].toUpperCase());
            String input = args[1];

            switch(mode) {
                case FONT:
                    convertFont(input);
                    break;
                case IMAGE:
                    convertImage(input);
                    break;
            }
        } catch(Exception ex) {
            System.err.println("Needs 2 arguments:\n   1) FONT/IMAGE\n   2) input file name");
            System.exit(0);
        }
    }

    private static void convertFont(String input) {
        try {
            BufferedImage img = ImageIO.read(new File(input));
            ArrayList<Integer> temp = new ArrayList<Integer>();
            String output = "";

            int widest = 0;

            for(int col = 0; col < img.getWidth(); col++) {
              int colData = 0;

              for(int row = img.getHeight() - 1; row >= 0; row--)
                colData = (colData << 1) | (!new Color(img.getRGB(col, row)).equals(Color.WHITE) ? 1 : 0);

              if(colData == 0) {
                if(temp.isEmpty())
                  continue;

                Collections.reverse(temp);
                widest = Math.max(widest, temp.size());

                output += "  { ";
                for(int i : temp)
                  output += String.format("0x%04X, ", i);
                output += "LETTER_TERM },\n";

                temp.clear();
              } else {
                temp.add(colData);
              }
            }
            
            output = output.substring(0, output.length() - 2) + "\n"; // final comma

            System.out.println("const uint16_t letterData[][" + (widest + 2) + "] = {");
            System.out.println("  { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, LETTER_TERM },");
            System.out.print(output);
            System.out.println("};");
        } catch (IOException ex) {
            System.err.println("IOException " + ex.getMessage());
            System.exit(0);
        }
    }

    private static void convertImage(String input) {

        try {
            Image originalImage = ImageIO.read(new File(input));

            // resize
            int newWidth = originalImage.getWidth(null);
            int newHeight = originalImage.getHeight(null);
            if(newWidth > NO_OF_COLS || newHeight > NO_OF_ROWS) {
                double aspectRatio = newWidth * 1.0 / newHeight;
                if(aspectRatio > (NO_OF_COLS * 1.0 / NO_OF_ROWS)) {
                    newWidth = NO_OF_COLS;
                    newHeight = (int) (NO_OF_COLS / aspectRatio);
                } else {
                    newWidth = (int) (NO_OF_ROWS * aspectRatio);
                    newHeight = NO_OF_ROWS;
                }
            }
            BufferedImage img = new BufferedImage(NO_OF_COLS, NO_OF_ROWS, BufferedImage.TYPE_INT_RGB);
            Graphics2D graphics = img.createGraphics();
            graphics.setColor(Color.BLACK);
            graphics.fillRect(0, 0, NO_OF_COLS, NO_OF_ROWS);
            graphics.setComposite(AlphaComposite.Src);
            graphics.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_BILINEAR);
            graphics.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY);
            graphics.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
            graphics.drawImage(originalImage, (NO_OF_COLS - newWidth) / 2, (NO_OF_ROWS - newHeight) / 2, newWidth, newHeight, null);
            graphics.dispose();
            ImageIO.write(img, "png", new File("temp.png"));

            // convert
            String outputRed = "PROGMEM const prog_uint8_t dataRed[NO_OF_COLUMNS][NO_OF_BYTES_PER_COLOUR] = {";
            String outputGreen = "PROGMEM const prog_uint8_t dataGreen[NO_OF_COLUMNS][NO_OF_BYTES_PER_COLOUR] = {";
            String outputBlue = "PROGMEM const prog_uint8_t dataBlue[NO_OF_COLUMNS][NO_OF_BYTES_PER_COLOUR] = {";

            for(int col = NO_OF_COLS - 1; col >= 0; col--) {
                outputRed += "\n{ ";
                outputGreen += "\n{ ";
                outputBlue += "\n{ ";
                for(int row = 0; row < NO_OF_ROWS; row++) {
                    Color colour = new Color(img.getRGB(col, row));
                    switch(row % 2) {
                        case 0:
                            outputRed += "0x" + Integer.toHexString(colour.getRed()) + ", ";
                            outputGreen += "0x" + Integer.toHexString(colour.getGreen()) + ", ";
                            outputBlue += "0x" + Integer.toHexString(colour.getBlue()) + ", ";
                            break;
                        default:
                            outputRed += "0x0" + Integer.toHexString(colour.getRed() >> 4) + ", ";
                            outputRed += "0x" + Integer.toHexString(colour.getRed() & 0x0F) + "0";
                            outputGreen += "0x0" + Integer.toHexString(colour.getGreen() >> 4) + ", ";
                            outputGreen += "0x" + Integer.toHexString(colour.getGreen() & 0x0F) + "0";
                            outputBlue += "0x0" + Integer.toHexString(colour.getBlue() >> 4) + ", ";
                            outputBlue += "0x" + Integer.toHexString(colour.getBlue() & 0x0F) + "0";
                            if(row < NO_OF_ROWS - 1) {
                                outputRed += ", ";
                                outputGreen += ", ";
                                outputBlue += ", ";
                            }
                            break;
                    }
                }
                outputRed += " }";
                outputGreen += " }";
                outputBlue += " }";
                if(col > 0) {
                    outputRed += ", ";
                    outputGreen += ", ";
                    outputBlue += ", ";
                }
            }
            outputRed += "\n};";
            outputGreen += "\n};";
            outputBlue += "\n};";

            System.out.println(outputRed + "\n\n" + outputGreen + "\n\n" + outputBlue);
        } catch (IOException ex) {
            System.err.println("IOException: " + ex.getMessage());
            System.exit(0);
        }
    }

}
