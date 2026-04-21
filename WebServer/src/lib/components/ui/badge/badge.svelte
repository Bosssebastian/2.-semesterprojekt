<script lang="ts" module>
	import { cn, type WithElementRef } from "$lib/utils.js";
	import type { HTMLAttributes } from "svelte/elements";
	import { type VariantProps, tv } from "tailwind-variants";

	export const badgeVariants = tv({
		base: "inline-flex items-center gap-1 rounded-full border px-3 py-1 text-xs font-semibold tracking-[0.18em] uppercase transition-colors",
		variants: {
			variant: {
				default: "border-primary/15 bg-primary/10 text-primary",
				secondary: "border-border/80 bg-secondary text-secondary-foreground",
				success: "border-emerald-500/20 bg-emerald-500/12 text-emerald-700",
				warning: "border-amber-500/25 bg-amber-500/12 text-amber-700",
				destructive: "border-destructive/20 bg-destructive/10 text-destructive",
				outline: "border-border/80 bg-background/80 text-foreground",
			},
		},
		defaultVariants: {
			variant: "default",
		},
	});

	export type BadgeVariant = VariantProps<typeof badgeVariants>["variant"];
	export type BadgeProps = WithElementRef<HTMLAttributes<HTMLDivElement>, HTMLDivElement> & {
		variant?: BadgeVariant;
	};
</script>

<script lang="ts">
	let {
		class: className,
		variant = "default",
		ref = $bindable(null),
		children,
		...restProps
	}: BadgeProps = $props();
</script>

<div
	bind:this={ref}
	data-slot="badge"
	class={cn(badgeVariants({ variant }), className)}
	{...restProps}
>
	{@render children?.()}
</div>
